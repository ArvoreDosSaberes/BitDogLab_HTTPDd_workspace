/**
 * =============================================================================
 * @file    log_vt100.c
 * @brief   Implementação do Sistema de Logging com Cores VT100/ANSI
 * @version 1.0.0
 * @date    2024
 * 
 * @project BitDogLab_HTTPDd_workspace
 * @url     https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace
 * 
 * @author  Carlos Delfino
 * @email   consultoria@carlosdelfino.eti.br
 * @website https://carlosdelfino.eti.br
 * @github  https://github.com/CarlosDelfino
 * 
 * @license CC BY 4.0 - https://creativecommons.org/licenses/by/4.0/
 * 
 * @details Este arquivo implementa todas as funções do sistema de logging,
 *          incluindo:
 *          - Formatação de strings com suporte a %b (binário)
 *          - Saída colorida para terminais VT100/ANSI
 *          - Filtragem de mensagens por nível
 * 
 *          ARQUITETURA DO MÓDULO:
 *          ┌─────────────────────────────────────────────────────────────┐
 *          │                      log_write()                            │
 *          │  (função principal, chamada pelas macros LOG_*)             │
 *          └─────────────────────────┬───────────────────────────────────┘
 *                                    │
 *                    ┌───────────────┴───────────────┐
 *                    ▼                               ▼
 *          ┌─────────────────────┐       ┌─────────────────────┐
 *          │   vsnprintf()       │       │  log_vsnprintf()    │
 *          │ (formato padrão)    │       │ (suporte a %b)      │
 *          └─────────────────────┘       └──────────┬──────────┘
 *                                                   │
 *                    ┌──────────────────────────────┼──────────────────┐
 *                    ▼                              ▼                  ▼
 *          ┌────────────────┐           ┌────────────────┐   ┌────────────────┐
 *          │ append_char()  │           │ append_int()   │   │ append_binary()│
 *          │ append_str()   │           │ append_uint()  │   │                │
 *          └────────────────┘           │ append_hex()   │   └────────────────┘
 *                                       └────────────────┘
 * 
 * @note    Este módulo é otimizado para sistemas embarcados com recursos
 *          limitados, evitando alocação dinâmica de memória.
 * =============================================================================
 */

#include "log_vt100.h"

#include <stdio.h>    /* Para printf, vsnprintf */
#include <stdarg.h>   /* Para va_list, va_start, va_end */
#include <stdint.h>   /* Para uintptr_t */

/* =============================================================================
 * SEÇÃO 1: VARIÁVEIS GLOBAIS ESTÁTICAS
 * =============================================================================
 */

/**
 * @var current_level
 * @brief Nível mínimo de log para exibição em tempo de execução
 * 
 * @details Esta variável controla qual o nível mínimo de mensagens que
 *          serão exibidas. Inicializada com LOG_DEFAULT_LEVEL (definido
 *          em log_vt100.h, padrão LOG_LEVEL_INFO).
 * 
 *          Mensagens com nível < current_level são descartadas.
 * 
 * @note    Esta variável é estática (visível apenas neste arquivo) para
 *          encapsulamento. Use log_set_level() para modificá-la.
 */
static log_level_t current_level = LOG_DEFAULT_LEVEL;

/* =============================================================================
 * SEÇÃO 2: FUNÇÕES AUXILIARES DE FORMATAÇÃO
 * =============================================================================
 * 
 * Estas funções são usadas pelo formatador personalizado log_vsnprintf()
 * para construir strings no buffer de saída. Todas são estáticas (privadas)
 * pois são detalhes de implementação.
 */

/**
 * @brief Adiciona um caractere ao buffer de saída
 * 
 * @details Esta é a função base de todas as operações de append.
 *          Verifica se há espaço no buffer antes de adicionar o caractere,
 *          garantindo que nunca ocorra buffer overflow.
 * 
 *          ALGORITMO:
 *          1. Verifica se (índice_atual + 1) < tamanho_buffer
 *             - O +1 é para reservar espaço para o '\0' final
 *          2. Se houver espaço, adiciona o caractere e incrementa índice
 *          3. Se não houver espaço, ignora silenciosamente (trunca)
 * 
 * @param buf   Ponteiro para o buffer de saída
 * @param size  Tamanho total do buffer (incluindo espaço para '\0')
 * @param idx   Ponteiro para o índice atual no buffer (será incrementado)
 * @param c     Caractere a ser adicionado
 * 
 * @note    O buffer DEVE ter espaço para pelo menos 1 caractere + '\0'
 * 
 * @example char buf[10]; size_t idx = 0;
 *          append_char(buf, 10, &idx, 'A');  // buf = "A", idx = 1
 */
static void append_char(char *buf, size_t size, size_t *idx, char c) {
    /* Passo 1: Verificar se há espaço no buffer */
    /* Precisamos de espaço para o caractere E para o '\0' final */
    if (*idx + 1 < size) {
        /* Passo 2: Adicionar o caractere na posição atual */
        buf[*idx] = c;
        
        /* Passo 3: Incrementar o índice para a próxima posição */
        (*idx)++;
    }
    /* Se não há espaço, o caractere é simplesmente ignorado (truncamento) */
}

/**
 * @brief Adiciona uma string ao buffer de saída
 * 
 * @details Copia caractere por caractere da string de origem para o
 *          buffer de saída, usando append_char() para cada um.
 * 
 *          TRATAMENTO DE NULL:
 *          Se a string de entrada for NULL, imprime "(null)" para
 *          indicar claramente o problema em vez de causar crash.
 * 
 * @param buf   Ponteiro para o buffer de saída
 * @param size  Tamanho total do buffer
 * @param idx   Ponteiro para o índice atual (será atualizado)
 * @param s     String a ser adicionada (pode ser NULL)
 * 
 * @example append_str(buf, 256, &idx, "Hello");
 *          append_str(buf, 256, &idx, NULL);  // Adiciona "(null)"
 */
static void append_str(char *buf, size_t size, size_t *idx, const char *s) {
    /* Passo 1: Tratar ponteiro NULL de forma segura */
    if (!s) {
        s = "(null)";
    }
    
    /* Passo 2: Copiar cada caractere até encontrar o terminador '\0' */
    while (*s) {
        append_char(buf, size, idx, *s++);
    }
}

/**
 * @brief Adiciona um inteiro sem sinal ao buffer em formato decimal
 * 
 * @details Converte um unsigned int para sua representação em string
 *          decimal e adiciona ao buffer.
 * 
 *          ALGORITMO DE CONVERSÃO:
 *          1. Caso especial: se valor é 0, adiciona '0' diretamente
 *          2. Caso geral: extrai dígitos do menos significativo para
 *             o mais significativo usando divisão por 10
 *          3. Os dígitos são armazenados em ordem reversa num buffer
 *             temporário, depois copiados em ordem correta
 * 
 *          EXEMPLO VISUAL (valor = 123):
 *          ┌─────────────────────────────────────────────────┐
 *          │ Iteração 1: 123 % 10 = 3, tmp = "3"             │
 *          │ Iteração 2: 12  % 10 = 2, tmp = "32"            │
 *          │ Iteração 3: 1   % 10 = 1, tmp = "321"           │
 *          │ Resultado reverso: "123"                         │
 *          └─────────────────────────────────────────────────┘
 * 
 * @param buf   Buffer de saída
 * @param size  Tamanho do buffer
 * @param idx   Índice atual (será atualizado)
 * @param v     Valor unsigned int a converter
 */
static void append_uint(char *buf, size_t size, size_t *idx, unsigned int v) {
    /* Buffer temporário para armazenar dígitos em ordem reversa */
    /* 16 caracteres é suficiente para qualquer unsigned int (máx ~10 dígitos) */
    char tmp[16];
    int t = 0;  /* Índice no buffer temporário */
    
    /* Passo 1: Caso especial - valor zero */
    if (v == 0) {
        tmp[t++] = '0';
    } else {
        /* Passo 2: Extrair dígitos em ordem reversa */
        while (v > 0 && t < (int)sizeof(tmp)) {
            /* Obtém o último dígito (resto da divisão por 10) */
            tmp[t++] = (char)('0' + (v % 10));
            
            /* Remove o último dígito (divisão inteira por 10) */
            v /= 10;
        }
    }
    
    /* Passo 3: Copiar dígitos em ordem correta (reverter) */
    while (t-- > 0) {
        append_char(buf, size, idx, tmp[t]);
    }
}

/**
 * @brief Adiciona um inteiro com sinal ao buffer em formato decimal
 * 
 * @details Similar a append_uint(), mas trata números negativos
 *          adicionando o sinal '-' antes do valor absoluto.
 * 
 *          TRATAMENTO DE NEGATIVOS:
 *          1. Se valor < 0, adiciona '-' ao buffer
 *          2. Converte para positivo e usa append_uint()
 * 
 * @param buf   Buffer de saída
 * @param size  Tamanho do buffer
 * @param idx   Índice atual (será atualizado)
 * @param v     Valor int (com sinal) a converter
 * 
 * @note    Cuidado com INT_MIN (-2147483648) que não pode ser negado
 *          em complemento de dois sem overflow. Esta implementação
 *          simplificada não trata esse caso extremo.
 */
static void append_int(char *buf, size_t size, size_t *idx, int v) {
    /* Passo 1: Verificar se o número é negativo */
    if (v < 0) {
        /* Passo 2a: Adicionar o sinal de menos */
        append_char(buf, size, idx, '-');
        
        /* Passo 2b: Converter para positivo */
        v = -v;
    }
    
    /* Passo 3: Usar append_uint para o valor absoluto */
    append_uint(buf, size, idx, (unsigned int)v);
}

/**
 * @brief Adiciona um valor hexadecimal ao buffer
 * 
 * @details Converte um unsigned int para representação hexadecimal.
 *          Suporta saída em maiúsculas (ABCDEF) ou minúsculas (abcdef).
 * 
 *          ALGORITMO:
 *          Similar a append_uint(), mas usa base 16 em vez de base 10.
 *          Usa operações bit-a-bit para eficiência:
 *          - v & 0xF: obtém os 4 bits menos significativos (1 dígito hex)
 *          - v >>= 4: remove os 4 bits menos significativos
 * 
 *          EXEMPLO (valor = 0xAB = 171):
 *          ┌─────────────────────────────────────────────────┐
 *          │ Iteração 1: 0xAB & 0xF = 0xB, tmp = "B"         │
 *          │ Iteração 2: 0x0A & 0xF = 0xA, tmp = "BA"        │
 *          │ Resultado reverso: "AB"                          │
 *          └─────────────────────────────────────────────────┘
 * 
 * @param buf   Buffer de saída
 * @param size  Tamanho do buffer
 * @param idx   Índice atual
 * @param v     Valor a converter
 * @param upper Se != 0, usa maiúsculas (A-F); se 0, usa minúsculas (a-f)
 */
static void append_hex(char *buf, size_t size, size_t *idx, unsigned int v, int upper) {
    /* Tabela de dígitos hexadecimais */
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    
    /* Buffer temporário para dígitos em ordem reversa */
    char tmp[16];
    int t = 0;
    
    /* Passo 1: Caso especial - valor zero */
    if (v == 0) {
        tmp[t++] = '0';
    } else {
        /* Passo 2: Extrair dígitos hexadecimais */
        while (v > 0 && t < (int)sizeof(tmp)) {
            /* Obtém os 4 bits menos significativos como índice na tabela */
            tmp[t++] = digits[v & 0xFU];
            
            /* Remove os 4 bits menos significativos (shift right) */
            v >>= 4;
        }
    }
    
    /* Passo 3: Copiar em ordem correta */
    while (t-- > 0) {
        append_char(buf, size, idx, tmp[t]);
    }
}

/**
 * @brief Adiciona um valor em formato binário ao buffer
 * 
 * @details Converte um unsigned int para sua representação binária.
 *          Omite zeros à esquerda para saída mais limpa.
 * 
 *          ALGORITMO:
 *          1. Itera do bit mais significativo (MSB) ao menos significativo (LSB)
 *          2. Omite zeros iniciais até encontrar o primeiro '1'
 *          3. Após o primeiro '1', imprime todos os bits
 *          4. Se o valor for 0, imprime apenas "0"
 * 
 *          EXEMPLO (valor = 42 = 0b101010):
 *          ┌──────────────────────────────────────────────────────────────┐
 *          │ Bits 31-6: todos 0, omitidos (started = 0)                   │
 *          │ Bit 5: 1, imprime "1" (started = 1)                          │
 *          │ Bit 4: 0, imprime "0"                                        │
 *          │ Bit 3: 1, imprime "1"                                        │
 *          │ Bit 2: 0, imprime "0"                                        │
 *          │ Bit 1: 1, imprime "1"                                        │
 *          │ Bit 0: 0, imprime "0"                                        │
 *          │ Resultado: "101010"                                          │
 *          └──────────────────────────────────────────────────────────────┘
 * 
 * @param buf   Buffer de saída
 * @param size  Tamanho do buffer
 * @param idx   Índice atual
 * @param value Valor a converter para binário
 */
static void append_binary(char *buf, size_t size, size_t *idx, unsigned int value) {
    /* Flag para indicar se já encontramos o primeiro bit '1' */
    int started = 0;
    
    /* Iterar do bit mais significativo (31 para 32-bit) ao menos significativo (0) */
    for (int i = (int)(sizeof(unsigned int) * 8 - 1); i >= 0; --i) {
        /* Extrair o bit na posição i usando shift e AND */
        unsigned int bit = (value >> i) & 1U;
        
        if (bit) {
            /* Bit é 1: marcar como iniciado e imprimir */
            started = 1;
            append_char(buf, size, idx, '1');
        } else if (started) {
            /* Bit é 0, mas já iniciamos: imprimir o zero */
            append_char(buf, size, idx, '0');
        }
        /* Se bit é 0 e ainda não iniciamos, pular (omitir zeros à esquerda) */
    }
    
    /* Caso especial: se o valor era 0, nada foi impresso */
    if (!started) {
        append_char(buf, size, idx, '0');
    }
}

/**
 * @brief Verifica se a string de formato contém o especificador %b
 * 
 * @details Esta função faz uma varredura rápida na string de formato
 *          para determinar se contém "%b" (binário). Isso é usado para
 *          decidir entre usar vsnprintf() padrão (mais eficiente) ou
 *          log_vsnprintf() personalizado (necessário para %b).
 * 
 *          OTIMIZAÇÃO:
 *          A maioria das mensagens de log não usa %b, então usar
 *          vsnprintf() padrão na maioria dos casos é mais eficiente.
 * 
 * @param fmt String de formato a verificar
 * 
 * @return 1 se a string contém "%b", 0 caso contrário
 */
static int format_has_binary(const char *fmt) {
    /* Percorrer a string procurando por "%b" */
    while (*fmt) {
        if (*fmt == '%' && *(fmt + 1) == 'b') {
            return 1;  /* Encontrou %b */
        }
        ++fmt;
    }
    return 0;  /* Não encontrou %b */
}

/* =============================================================================
 * SEÇÃO 3: FORMATADOR PERSONALIZADO
 * =============================================================================
 */

/**
 * @brief Formatador personalizado com suporte a %b (binário)
 * 
 * @details Esta função é uma implementação simplificada de vsnprintf()
 *          que adiciona suporte ao especificador não-padrão %b para
 *          impressão de valores em binário.
 * 
 *          ESPECIFICADORES SUPORTADOS:
 *          ┌──────┬────────────────────────────────────────────────────┐
 *          │ Spec │ Descrição                                          │
 *          ├──────┼────────────────────────────────────────────────────┤
 *          │ %%   │ Caractere '%' literal                              │
 *          │ %c   │ Caractere único                                    │
 *          │ %s   │ String (terminada em '\0')                         │
 *          │ %d   │ Inteiro decimal com sinal                          │
 *          │ %i   │ Inteiro decimal com sinal (sinônimo de %d)         │
 *          │ %u   │ Inteiro decimal sem sinal                          │
 *          │ %x   │ Hexadecimal minúsculo (a-f)                        │
 *          │ %X   │ Hexadecimal maiúsculo (A-F)                        │
 *          │ %p   │ Ponteiro (formato 0xNNNNNNNN)                      │
 *          │ %b   │ Binário (extensão não-padrão)                      │
 *          └──────┴────────────────────────────────────────────────────┘
 * 
 *          LIMITAÇÕES:
 *          - Não suporta modificadores de largura (ex: %5d)
 *          - Não suporta precisão (ex: %.2f)
 *          - Não suporta %f (float/double)
 *          - Não suporta modificadores de tamanho (ex: %ld, %lld)
 * 
 * @param out   Buffer de saída onde a string formatada será escrita
 * @param size  Tamanho do buffer de saída (incluindo espaço para '\0')
 * @param fmt   String de formato com especificadores
 * @param ap    Lista de argumentos variádicos (já inicializada com va_start)
 * 
 * @note    O buffer de saída sempre será terminado com '\0', mesmo se
 *          a mensagem for truncada.
 */
static void log_vsnprintf(char *out, size_t size, const char *fmt, va_list ap) {
    /* Índice atual de escrita no buffer de saída */
    size_t idx = 0;

    /* Processar cada caractere da string de formato */
    while (*fmt) {
        /* Passo 1: Verificar se é um caractere literal (não '%') */
        if (*fmt != '%') {
            /* Caractere normal, copiar diretamente */
            append_char(out, size, &idx, *fmt++);
            continue;
        }

        /* Passo 2: Encontramos '%', avançar para o especificador */
        ++fmt;
        
        /* Passo 3: Verificar escape '%%' (% literal) */
        if (*fmt == '%') {
            append_char(out, size, &idx, '%');
            ++fmt;
            continue;
        }

        /* Passo 4: Processar o especificador de formato */
        char spec = *fmt++;
        switch (spec) {
            case 'c': {
                /* %c: Caractere único */
                /* Nota: char é promovido para int em va_arg */
                int c = va_arg(ap, int);
                append_char(out, size, &idx, (char)c);
                break;
            }
            case 's': {
                /* %s: String */
                const char *s = va_arg(ap, const char *);
                append_str(out, size, &idx, s);
                break;
            }
            case 'd':
            case 'i': {
                /* %d ou %i: Inteiro decimal com sinal */
                int v = va_arg(ap, int);
                append_int(out, size, &idx, v);
                break;
            }
            case 'u': {
                /* %u: Inteiro decimal sem sinal */
                unsigned int v = va_arg(ap, unsigned int);
                append_uint(out, size, &idx, v);
                break;
            }
            case 'x': {
                /* %x: Hexadecimal minúsculo */
                unsigned int v = va_arg(ap, unsigned int);
                append_hex(out, size, &idx, v, 0);
                break;
            }
            case 'X': {
                /* %X: Hexadecimal maiúsculo */
                unsigned int v = va_arg(ap, unsigned int);
                append_hex(out, size, &idx, v, 1);
                break;
            }
            case 'p': {
                /* %p: Ponteiro - formato 0xNNNNNNNN */
                void *ptr = va_arg(ap, void *);
                unsigned int v = (unsigned int)(uintptr_t)ptr;
                append_str(out, size, &idx, "0x");
                append_hex(out, size, &idx, v, 0);
                break;
            }
            case 'b': {
                /* %b: Binário (extensão não-padrão) */
                unsigned int v = va_arg(ap, unsigned int);
                append_binary(out, size, &idx, v);
                break;
            }
            default:
                /* Especificador desconhecido: imprimir literalmente */
                /* Isso ajuda a debugar erros de formatação */
                append_char(out, size, &idx, '%');
                append_char(out, size, &idx, spec);
                break;
        }
    }

    /* Passo 5: Garantir terminação com '\0' */
    if (size > 0) {
        /* Se idx está dentro do buffer, terminar em idx */
        /* Se idx >= size (truncado), terminar na última posição válida */
        out[(idx < size) ? idx : (size - 1)] = '\0';
    }
}

/* =============================================================================
 * SEÇÃO 4: FUNÇÕES PÚBLICAS DA API
 * =============================================================================
 */

/**
 * @brief Define o nível mínimo de log em tempo de execução
 * 
 * @details Altera a variável estática current_level que controla
 *          quais mensagens são exibidas. Mensagens com nível inferior
 *          ao configurado serão descartadas por log_write().
 * 
 *          EXEMPLO DE USO:
 *          ┌────────────────────────────────────────────────────────────┐
 *          │ log_set_level(LOG_LEVEL_TRACE);  // Mostra tudo           │
 *          │ log_set_level(LOG_LEVEL_INFO);   // Mostra INFO e WARN    │
 *          │ log_set_level(LOG_LEVEL_WARN);   // Mostra apenas WARN    │
 *          └────────────────────────────────────────────────────────────┘
 * 
 * @param level Novo nível mínimo de log (log_level_t)
 */
void log_set_level(log_level_t level) {
    current_level = level;
}

/**
 * @brief Função principal de escrita de log com cores VT100
 * 
 * @details Esta é a função chamada por todas as macros LOG_*.
 *          Ela realiza as seguintes operações:
 * 
 *          FLUXO DE EXECUÇÃO:
 *          ┌─────────────────────────────────────────────────────────────┐
 *          │ 1. Verificar se nível >= current_level (filtro runtime)    │
 *          │    └─ Se não, retornar imediatamente (mensagem descartada) │
 *          │                                                             │
 *          │ 2. Selecionar código de cor VT100 baseado no nível         │
 *          │    ├─ TRACE: \x1b[90m (cinza)                              │
 *          │    ├─ DEBUG: \x1b[34m (azul)                               │
 *          │    ├─ INFO:  \x1b[32m (verde)                              │
 *          │    └─ WARN:  \x1b[33m (amarelo)                            │
 *          │                                                             │
 *          │ 3. Formatar a mensagem do usuário                          │
 *          │    ├─ Se contém %b: usar log_vsnprintf() personalizado     │
 *          │    └─ Se não: usar vsnprintf() padrão (mais eficiente)     │
 *          │                                                             │
 *          │ 4. Selecionar prefixo baseado no nível                     │
 *          │    Ex: "[INFO ] ", "[WARN ] ", etc.                        │
 *          │                                                             │
 *          │ 5. Imprimir: COR + PREFIXO + MENSAGEM + RESET + '\n'       │
 *          └─────────────────────────────────────────────────────────────┘
 * 
 *          CÓDIGOS VT100/ANSI:
 *          Os códigos de escape ANSI seguem o formato \x1b[Nm onde:
 *          - \x1b (ou \033) é o caractere ESC
 *          - [ inicia a sequência de controle
 *          - N é o código da cor/atributo
 *          - m termina a sequência
 * 
 *          CORES USADAS:
 *          - 0m:  Reset (volta ao padrão)
 *          - 32m: Verde (foreground)
 *          - 33m: Amarelo (foreground)
 *          - 34m: Azul (foreground)
 *          - 90m: Cinza brilhante (foreground)
 * 
 * @param level Nível de severidade da mensagem
 * @param fmt   String de formato (estilo printf, com suporte a %b)
 * @param ...   Argumentos variádicos
 */
void log_write(log_level_t level, const char *fmt, ...) {
    /* ========== PASSO 1: FILTRAGEM POR NÍVEL ========== */
    /* Verificar se a mensagem deve ser exibida baseado no nível */
    if (level < current_level) {
        /* Nível abaixo do mínimo configurado, descartar mensagem */
        return;
    }

    /* ========== PASSO 2: SELEÇÃO DE COR VT100 ========== */
    /* Códigos de cor VT100/ANSI para saída colorida no terminal */
    const char *color_reset = "\x1b[0m";  /* Reset para cor padrão */
    const char *color_code = "";           /* Cor específica do nível */

    switch (level) {
        case LOG_LEVEL_TRACE:
            color_code = "\x1b[90m";  /* Cinza (brilhante) - pouco visível */
            break;
        case LOG_LEVEL_DEBUG:
            color_code = "\x1b[34m";  /* Azul - informação de debug */
            break;
        case LOG_LEVEL_INFO:
            color_code = "\x1b[32m";  /* Verde - operação normal */
            break;
        case LOG_LEVEL_WARN:
            color_code = "\x1b[33m";  /* Amarelo - atenção */
            break;
        default:
            color_code = "\x1b[0m";   /* Padrão se nível desconhecido */
            break;
    }

    /* ========== PASSO 3: FORMATAÇÃO DA MENSAGEM ========== */
    /* Buffer para a mensagem formatada (256 bytes é suficiente para maioria) */
    char msg[256];
    
    /* Inicializar lista de argumentos variádicos */
    va_list ap;
    va_start(ap, fmt);
    
    /* Escolher o formatador apropriado */
    if (format_has_binary(fmt)) {
        /* String contém %b, usar formatador personalizado */
        log_vsnprintf(msg, sizeof msg, fmt, ap);
    } else {
        /* Usar vsnprintf padrão (mais eficiente) */
        vsnprintf(msg, sizeof msg, fmt, ap);
    }
    
    va_end(ap);

    /* ========== PASSO 4: SELEÇÃO DO PREFIXO ========== */
    /* Prefixo indica o nível da mensagem de forma textual */
    const char *prefix;
    switch (level) {
        case LOG_LEVEL_TRACE:
            prefix = "[TRACE] ";
            break;
        case LOG_LEVEL_DEBUG:
            prefix = "[DEBUG] ";
            break;
        case LOG_LEVEL_INFO:
            prefix = "[INFO ] ";  /* Espaço extra para alinhamento */
            break;
        case LOG_LEVEL_WARN:
            prefix = "[WARN ] ";  /* Espaço extra para alinhamento */
            break;
        default:
            prefix = "[LOG  ] ";
            break;
    }

    /* ========== PASSO 5: SAÍDA FINAL ========== */
    /* Formato: COR + PREFIXO + MENSAGEM + RESET + NEWLINE */
    printf("%s%s%s%s\n", color_code, prefix, msg, color_reset);
}

