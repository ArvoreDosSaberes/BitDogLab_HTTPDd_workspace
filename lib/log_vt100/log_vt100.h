/**
 * =============================================================================
 * @file    log_vt100.h
 * @brief   Sistema de Logging com Suporte a Cores VT100/ANSI
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
 * @details Este módulo implementa um sistema de logging leve e eficiente
 *          para sistemas embarcados, com as seguintes características:
 * 
 *          FUNCIONALIDADES:
 *          - Níveis de log hierárquicos (TRACE, DEBUG, INFO, WARN)
 *          - Saída colorida usando códigos de escape VT100/ANSI
 *          - Filtragem em tempo de compilação (reduz tamanho do binário)
 *          - Filtragem em tempo de execução (flexibilidade)
 *          - Suporte ao especificador %b para impressão binária
 *          - Thread-safe para uso com FreeRTOS
 * 
 *          HIERARQUIA DE NÍVEIS:
 *          ┌─────────┬─────────┬─────────────────────────────────────────┐
 *          │ Nível   │ Valor   │ Uso Recomendado                         │
 *          ├─────────┼─────────┼─────────────────────────────────────────┤
 *          │ TRACE   │ 0       │ Detalhes finos, loops internos          │
 *          │ DEBUG   │ 1       │ Informações de depuração                │
 *          │ INFO    │ 2       │ Eventos importantes do sistema          │
 *          │ WARN    │ 3       │ Avisos de condições anormais            │
 *          └─────────┴─────────┴─────────────────────────────────────────┘
 * 
 *          CORES NO TERMINAL:
 *          - TRACE: Cinza     (pouco visível, para mensagens verbosas)
 *          - DEBUG: Azul      (destaca informações de debug)
 *          - INFO:  Verde     (indica operação normal)
 *          - WARN:  Amarelo   (chama atenção para avisos)
 * 
 * @note    Requer terminal com suporte a códigos ANSI/VT100
 * 
 * @example Uso básico:
 *          LOG_INFO("Sistema iniciado com sucesso");
 *          LOG_DEBUG("Valor da variável: %d", valor);
 *          LOG_WARN("Temperatura alta: %d°C", temp);
 *          LOG_TRACE("Entrando na função %s", __func__);
 * =============================================================================
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>   /* Para va_list em funções variádicas */
#include <stdint.h>   /* Para tipos inteiros de tamanho fixo */

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * SEÇÃO 1: ENUMERAÇÃO DE NÍVEIS DE LOG
 * =============================================================================
 * 
 * Os níveis de log seguem uma hierarquia onde valores menores representam
 * mensagens mais detalhadas (verbosas) e valores maiores representam
 * mensagens mais críticas.
 * 
 * A filtragem funciona assim: se o nível configurado é INFO (2), apenas
 * mensagens de nível INFO (2) ou superior (WARN=3) serão exibidas.
 * Mensagens DEBUG (1) e TRACE (0) serão descartadas.
 */

/**
 * @enum log_level_t
 * @brief Níveis de severidade para mensagens de log
 * 
 * @details Esta enumeração define os níveis de log disponíveis no sistema.
 *          A ordem é importante: níveis com valor menor são mais verbosos.
 * 
 * @var LOG_LEVEL_TRACE
 *      Nível 0 - Mais verboso. Use para:
 *      - Rastrear fluxo de execução
 *      - Valores de variáveis em loops
 *      - Entrada/saída de funções
 *      Cor: Cinza
 * 
 * @var LOG_LEVEL_DEBUG
 *      Nível 1 - Informações de depuração. Use para:
 *      - Estados internos importantes
 *      - Valores de configuração
 *      - Diagnóstico de problemas
 *      Cor: Azul
 * 
 * @var LOG_LEVEL_INFO
 *      Nível 2 - Informações operacionais. Use para:
 *      - Início/fim de operações importantes
 *      - Conexões estabelecidas
 *      - Eventos significativos do sistema
 *      Cor: Verde
 * 
 * @var LOG_LEVEL_WARN
 *      Nível 3 - Avisos. Use para:
 *      - Condições inesperadas não fatais
 *      - Recursos esgotando
 *      - Falhas recuperáveis
 *      Cor: Amarelo
 */
typedef enum {
    LOG_LEVEL_TRACE = 0,  /* Mensagens muito detalhadas, depuração fina */
    LOG_LEVEL_DEBUG = 1,  /* Informações de depuração gerais */
    LOG_LEVEL_INFO  = 2,  /* Mensagens informativas de alto nível */
    LOG_LEVEL_WARN  = 3,  /* Avisos sobre condições inesperadas */
} log_level_t;

/* =============================================================================
 * SEÇÃO 2: FUNÇÕES DA API DE LOGGING
 * =============================================================================
 */

/**
 * @brief Define o nível mínimo de log em tempo de execução
 * 
 * @details Esta função permite alterar dinamicamente o nível de log
 *          durante a execução do programa. Mensagens com nível abaixo
 *          do configurado serão descartadas silenciosamente.
 * 
 * @param level Nível mínimo de log a ser exibido (log_level_t)
 * 
 * @note    A filtragem em tempo de execução tem um pequeno overhead,
 *          pois a função log_write() é chamada antes da verificação.
 *          Para eliminar completamente o overhead, use LOG_LEVEL em
 *          tempo de compilação.
 * 
 * @example log_set_level(LOG_LEVEL_DEBUG);  // Mostra DEBUG, INFO, WARN
 *          log_set_level(LOG_LEVEL_WARN);   // Mostra apenas WARN
 */
void log_set_level(log_level_t level);

/**
 * @brief Função principal de escrita de log
 * 
 * @details Escreve uma mensagem de log formatada no stdout com:
 *          - Cor VT100 apropriada para o nível
 *          - Prefixo indicando o nível ([TRACE], [DEBUG], etc.)
 *          - Mensagem formatada no estilo printf
 *          - Reset de cor ao final
 * 
 * @param level Nível de severidade da mensagem
 * @param fmt   String de formato estilo printf, com suporte adicional a:
 *              - %b: Imprime valor em binário (extensão não-padrão)
 *              - %d, %i: Inteiros decimais com sinal
 *              - %u: Inteiros decimais sem sinal
 *              - %x, %X: Hexadecimal minúsculo/maiúsculo
 *              - %s: Strings
 *              - %c: Caracteres
 *              - %p: Ponteiros
 * @param ...   Argumentos variádicos correspondentes ao formato
 * 
 * @note    O buffer interno é limitado a 256 caracteres. Mensagens
 *          maiores serão truncadas.
 * 
 * @warning Esta função não é thread-safe por padrão. Em ambientes
 *          multi-thread, considere usar um mutex ao redor das chamadas.
 */
void log_write(log_level_t level, const char *fmt, ...);

/* =============================================================================
 * SEÇÃO 3: CONSTANTES E CONFIGURAÇÕES DE COMPILAÇÃO
 * =============================================================================
 */

/**
 * @def LOG_DEFAULT_LEVEL
 * @brief Nível padrão de log em tempo de execução
 * 
 * @details Define o nível inicial do filtro de log quando o sistema
 *          é iniciado. Pode ser alterado posteriormente com log_set_level().
 * 
 * @note    Padrão: LOG_LEVEL_INFO (mostra INFO e WARN)
 *          Pode ser redefinido antes de incluir este header ou via CMake
 */
#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_LEVEL_INFO
#endif

/**
 * @def LOG_LEVEL
 * @brief Nível de log em tempo de compilação
 * 
 * @details Esta macro controla quais chamadas de log são compiladas
 *          no binário final. Diferente de LOG_DEFAULT_LEVEL que filtra
 *          em runtime, esta remove completamente o código não utilizado.
 * 
 *          VALORES E EFEITOS:
 *          ┌────────┬──────────────────────────────────────────────────┐
 *          │ Valor  │ Macros Ativas                                    │
 *          ├────────┼──────────────────────────────────────────────────┤
 *          │ -1     │ Nenhuma (todas viram NOP, zero overhead)         │
 *          │  0     │ Apenas LOG_WARN                                  │
 *          │  1     │ LOG_INFO + LOG_WARN                              │
 *          │  2     │ LOG_DEBUG + LOG_INFO + LOG_WARN                  │
 *          │  3     │ LOG_TRACE + LOG_DEBUG + LOG_INFO + LOG_WARN      │
 *          └────────┴──────────────────────────────────────────────────┘
 * 
 * @note    Use -1 para builds de produção onde logging não é necessário
 *          Use 3 para desenvolvimento/debug completo
 */
#ifndef LOG_LEVEL
#define LOG_LEVEL 1  /* Padrão: INFO + WARN */
#endif

/**
 * @def LOG_TAG
 * @brief Tag opcional para identificar o módulo/subsistema
 * 
 * @details Pode ser definido por módulo para prefixar mensagens.
 *          Atualmente não utilizado na formatação, reservado para
 *          expansões futuras.
 * 
 * @example #define LOG_TAG "WiFi"
 *          // Poderia gerar: [INFO][WiFi] Conectado
 */
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

/* =============================================================================
 * SEÇÃO 4: MACROS DE LOGGING
 * =============================================================================
 * 
 * Estas macros fornecem uma interface conveniente para o sistema de log,
 * com filtragem automática em tempo de compilação baseada em LOG_LEVEL.
 */

/**
 * @def LOG(level, fmt, ...)
 * @brief Macro genérica de logging
 * 
 * @details Converte o nível simbólico (TRACE, DEBUG, INFO, WARN) para
 *          o valor correspondente do enum e chama log_write().
 * 
 * @param level Nome do nível SEM prefixo LOG_LEVEL_ (ex: INFO, não LOG_LEVEL_INFO)
 * @param fmt   String de formato
 * @param ...   Argumentos variádicos
 * 
 * @example LOG(INFO, "Valor: %d", x);  // Equivale a log_write(LOG_LEVEL_INFO, ...)
 */
#define LOG(level, fmt, ...) \
    log_write(LOG_LEVEL_##level, fmt, ##__VA_ARGS__)

/*
 * FILTRAGEM EM TEMPO DE COMPILAÇÃO
 * 
 * As macros abaixo são condicionalmente definidas baseadas em LOG_LEVEL.
 * Se um nível está desabilitado, a macro correspondente se torna ((void)0),
 * que é uma expressão vazia que não gera código no binário final.
 * 
 * Isso permite deixar chamadas de LOG_DEBUG no código de produção sem
 * qualquer impacto de performance quando LOG_LEVEL < 2.
 */

#if LOG_LEVEL < 0
/* ==========================================================================
 * LOG_LEVEL = -1: MODO SILENCIOSO
 * Todas as macros são NOPs (no-operation), nenhum código de log é gerado.
 * Útil para builds de produção com requisitos rígidos de tamanho/performance.
 * ========================================================================== */
#define LOG_TRACE(fmt, ...) ((void)0)
#define LOG_DEBUG(fmt, ...) ((void)0)
#define LOG_INFO(fmt, ...)  ((void)0)
#define LOG_WARN(fmt, ...)  ((void)0)

#else
/* ==========================================================================
 * LOG_LEVEL >= 0: LOGGING HABILITADO
 * Macros são habilitadas seletivamente baseado no nível configurado.
 * ========================================================================== */

/**
 * @def LOG_TRACE(fmt, ...)
 * @brief Macro para mensagens de nível TRACE (mais verboso)
 * 
 * @details Habilitada apenas quando LOG_LEVEL >= 3.
 *          Use para rastreamento detalhado de execução.
 * 
 * @param fmt String de formato estilo printf
 * @param ... Argumentos variádicos
 * 
 * @example LOG_TRACE("Entrando em %s, linha %d", __func__, __LINE__);
 */
#if LOG_LEVEL >= 3
#define LOG_TRACE(fmt, ...) LOG(TRACE, fmt, ##__VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...) ((void)0)
#endif

/**
 * @def LOG_DEBUG(fmt, ...)
 * @brief Macro para mensagens de nível DEBUG
 * 
 * @details Habilitada apenas quando LOG_LEVEL >= 2.
 *          Use para informações úteis durante desenvolvimento.
 * 
 * @param fmt String de formato estilo printf
 * @param ... Argumentos variádicos
 * 
 * @example LOG_DEBUG("Buffer: ptr=%p, size=%u", buf, size);
 */
#if LOG_LEVEL >= 2
#define LOG_DEBUG(fmt, ...) LOG(DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

/**
 * @def LOG_INFO(fmt, ...)
 * @brief Macro para mensagens de nível INFO
 * 
 * @details Habilitada apenas quando LOG_LEVEL >= 1.
 *          Use para eventos operacionais importantes.
 * 
 * @param fmt String de formato estilo printf
 * @param ... Argumentos variádicos
 * 
 * @example LOG_INFO("WiFi conectado, IP: %s", ip_addr);
 */
#if LOG_LEVEL >= 1
#define LOG_INFO(fmt, ...)  LOG(INFO,  fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  ((void)0)
#endif

/**
 * @def LOG_WARN(fmt, ...)
 * @brief Macro para mensagens de nível WARN
 * 
 * @details Sempre habilitada quando LOG_LEVEL >= 0.
 *          Use para condições anormais que não impedem a execução.
 * 
 * @param fmt String de formato estilo printf
 * @param ... Argumentos variádicos
 * 
 * @example LOG_WARN("Falha ao conectar, tentando novamente...");
 */
#define LOG_WARN(fmt, ...)  LOG(WARN,  fmt, ##__VA_ARGS__)

#endif /* LOG_LEVEL < 0 */

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */

