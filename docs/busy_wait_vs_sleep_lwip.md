# Por que usar `busy_wait_ms` em vez de `sleep_ms` no contexto LwIP

## Resumo

Em aplicações que utilizam **LwIP (Lightweight IP)** com o Pico SDK, é preferível usar `busy_wait_ms()` em vez de `sleep_ms()` para delays curtos, especialmente dentro de callbacks ou loops de processamento de rede.

## O Problema com `sleep_ms`

A função `sleep_ms()` do Pico SDK coloca o processador em modo de baixo consumo (sleep) enquanto aguarda. Durante esse período:

1. **Interrupções podem ser perdidas ou atrasadas** — O LwIP depende de timers e callbacks para processar pacotes de rede.
2. **O polling de rede é interrompido** — Em configurações `NO_SYS=1` (sem RTOS), o LwIP precisa que `cyw43_arch_poll()` seja chamado frequentemente.
3. **Timeouts de TCP/IP podem falhar** — O stack TCP mantém timers internos que precisam ser atualizados regularmente.

## Por que `busy_wait_ms` funciona melhor

A função `busy_wait_ms()` realiza uma espera ativa (busy-wait), mantendo o processador ocupado:

```c
// Exemplo: delay sem bloquear o processamento LwIP
busy_wait_ms(10);
```

### Vantagens

- **Não interfere com interrupções** — O processador continua respondendo a IRQs.
- **Mantém o contexto de execução** — Ideal para delays curtos dentro de rotinas críticas.
- **Previsibilidade** — O tempo de espera é mais preciso em contextos onde o scheduler não está ativo.

## Quando usar cada um

| Situação | Recomendação |
|----------|--------------|
| Delay curto em callback HTTP/TCP | `busy_wait_ms()` ou `busy_wait_us()` |
| Delay longo (>100ms) fora do loop principal | `sleep_ms()` com cuidado |
| Loop principal com polling LwIP | Evitar ambos; usar timers |
| Aguardar conexão WiFi | `cyw43_arch_poll()` em loop com `busy_wait_ms()` |

## Exemplo prático

```c
// ❌ Evitar: pode causar timeout ou perda de pacotes
void minha_funcao_rede() {
    sleep_ms(50);  // Bloqueia o processamento
    // ...
}

// ✅ Preferir: mantém responsividade
void minha_funcao_rede() {
    busy_wait_ms(50);  // Espera ativa, IRQs continuam funcionando
    // ...
}

// ✅ Melhor ainda: usar polling incremental
void minha_funcao_rede() {
    for (int i = 0; i < 50; i++) {
        cyw43_arch_poll();  // Processa pacotes pendentes
        busy_wait_ms(1);
    }
}
```

## Considerações de energia

O `busy_wait_ms()` consome mais energia que `sleep_ms()`, pois o processador fica ativo. Para aplicações alimentadas por bateria, considere:

- Usar `sleep_ms()` apenas em momentos seguros (fora de operações de rede ativas)
- Implementar um sistema de estados que alterne entre modos de baixo consumo e processamento ativo

## Referências

- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [LwIP Wiki](https://lwip.fandom.com/wiki/LwIP_Wiki)
- [pico-examples/pico_w](https://github.com/raspberrypi/pico-examples/tree/master/pico_w)
