# Space Trash - Projeto B - Guia de Implementação

**Prazo:** 9 de Janeiro 2026, 19h00  
**Entrega:** Ficheiro ZIP no FENIX com pastas separadas + Makefiles

---

## 📋 Visão Geral do Projeto B

O Projeto B expande o Projeto A com um sistema distribuído completo:

- 3 componentes independentes que comunicam entre si
- Sistema de física aplicado às trash-ships
- Geração periódica de lixo
- Dashboard de estatísticas
- Suporte para múltiplos clientes simultâneos

---

## 🔧 Componentes a Desenvolver

### 1. new-universe-server

**Substitui:** universe-simulator + universe-server da Parte A

**Funcionalidades principais:**
- Simula todo o universo (planetas + lixo + trash-ships)
- Aplica regras de física a trash-ships
- Gera lixo periodicamente (10s)
- Muda planeta de reciclagem periodicamente (30s)
- Gere múltiplos clientes conectados
- Atualiza display a 30Hz (33ms)
- Aplica física a 100Hz (10ms)

**Tarefas:**
- [ ] Criar estrutura básica do servidor
- [ ] Implementar sockets ZMQ (REQ/REP + PUB)
- [ ] Adicionar data structures para trash-ships
- [ ] Implementar loop de física (10ms)
- [ ] Implementar geração periódica de lixo (10s)
- [ ] Implementar mudança de planeta reciclador (30s)
- [ ] Implementar display update (33ms)
- [ ] Adicionar tratamento de desconexões

**Operação do servidor:**
```
Inicialização
├── Ler configurações
├── Criar universo (planetas + lixo inicial)
└── Criar sockets ZMQ

Loop principal (concorrente):
├── Thread: Gestão de clientes
│   ├── Aceitar conexões
│   └── Receber comandos de movimento
├── Thread: Atualização do universo (física - 10ms)
│   ├── Aplicar física ao lixo
│   └── Aplicar física às trash-ships
├── Thread: Comunicação
│   └── Enviar estado do universo (PUB)
├── Thread Main: Display (SDL - 33ms)
│   └── Desenhar universo
└── Verificação do fim do universo
```

---

### 2. new-trash-ship-client

**Substitui:** universe-client da Parte A

**Funcionalidades:**
- Conecta ao servidor via ZMQ
- Captura teclas do teclado (SDL)
- Envia comandos de movimento (thrust)
- Recebe e exibe estado completo do universo
- Display idêntico ao servidor

**Tarefas:**
- [ ] Criar janela SDL para visualização
- [ ] Implementar leitura de teclas
- [ ] Implementar socket cliente ZMQ
- [ ] Receber e processar estado do universo
- [ ] Desenhar universo completo (planetas, lixo, ships)
- [ ] Sincronizar display com servidor

**Operação do cliente:**
```
Inicialização
├── Ler configurações
├── Conectar ao servidor (ZMQ REQ)
├── Subscribe para updates (ZMQ SUB)
└── Criar janela SDL

Loop principal (concorrente):
├── Thread: Leitura de teclas (SDL_KEYDOWN/KEYUP)
│   └── Enviar comandos ao servidor
├── Thread: Receber updates do universo
│   └── Atualizar estado local
└── Thread Main: Display (SDL)
    └── Desenhar universo
```

---

### 3. new-dashboard

**Nova aplicação independente** (Python ou Java)

**Exibe estatísticas:**
- Lixo reciclado por planeta
- Cargo atual de cada trash-ship
- Total de lixo no universo
- Proximidade do fim do universo

**Não requer SDL** - apenas output de texto

**Exemplo de output:**
```
=== SPACE TRASH DASHBOARD ===
Universe Status: 15/20 trash (75% capacity)
WARNING: Approaching collapse!

Planet A: 45 trash recycled
Planet B: 32 trash recycled
Planet C: 28 trash recycled

Ship A: 3/5 cargo
Ship B: 0/5 cargo (empty)
Ship C: 5/5 cargo (FULL)

Recycling Planet: B
```

**Tarefas:**
- [ ] Escolher linguagem (Python recomendado)
- [ ] Implementar socket ZMQ subscriber
- [ ] Processar mensagens do servidor
- [ ] Formatar e exibir estatísticas
- [ ] Atualizar display em tempo real

---

## ⚛️ Sistema de Física

### Constantes
```c
planet_mass = 10;           // mass units
trash_mass = 1;             // mass units
trash_ship_mass = 1;        // mass units (independente do cargo)
time_unit = 10;             // ms
gravitational_constant = 1;
friction = 0.99;            // 1% redução por time unit
```

### Física das Trash-Ships

**IMPORTANTE:** Trash-ships agora têm:
- **Inertia** - continuam em movimento mesmo sem comandos
- **Velocity** afetada por gravidade
- **Friction** - reduz 1% velocidade por frame
- **Thrust** dos motores - comandos do jogador

⚠️ **Comandos NÃO afetam posição diretamente - afetam velocidade!**

### Implementação

As mesmas 3 funções da física do lixo aplicam-se às ships:

```c
// 1. Calcular aceleração (gravidade)
void new_trash_acceleration(planet_structure planets[], int total_planets,
                            trash_structure trash[], int total_trash);

// 2. Atualizar velocidade (aceleração + friction)
void new_trash_velocity(trash_structure trash[], int total_trash);

// 3. Atualizar posição
void new_trash_position(trash_structure trash[], int total_trash);
```

### Thrust (Comandos do Jogador)

**Solução Simples (Recomendada):**
- Capturar eventos SDL_KEYDOWN
- Enviar mensagem de thrust ao servidor
- Servidor aplica força instantânea na direção correspondente
- Força gera aceleração → modifica velocidade

**Solução Avançada (Opcional):**
- Capturar SDL_KEYDOWN e SDL_KEYUP
- Enviar "pedal down" / "pedal up"
- Servidor aplica força contínua enquanto tecla pressionada

**Tarefas:**
- [ ] Adaptar funções de física para trash-ships
- [ ] Implementar thrust como força instantânea
- [ ] Aplicar friction às ships
- [ ] Testar movimento realista com inércia

---

## ⏱️ Operações Periódicas

Diferentes frequências para diferentes tarefas:

### 1. Física: **10ms (100Hz)**
- Aplicar aceleração, velocidade, posição
- Para trash **E** trash-ships
- **Mais crítico** - define a fluidez do jogo

### 2. Display: **33ms (30Hz)**
- Atualizar janela SDL
- Enviar estado aos clientes
- Menos frequente que física para performance

### 3. Geração de lixo: **10s (0.1Hz)**
- Criar novo lixo em posição aleatória
- **Apenas quando há trash-ships ativos**

### 4. Mudança de reciclador: **30s**
- Escolher novo planeta aleatoriamente
- Notificar todos os clientes

**Implementação sugerida:**
```c
// Usar timers separados ou timestamps
typedef struct {
    uint64_t last_physics;      // 10ms
    uint64_t last_display;      // 33ms
    uint64_t last_trash_gen;    // 10s
    uint64_t last_recycler;     // 30s
} timers_t;

// No loop principal
while (running) {
    uint64_t now = get_time_ms();
    
    if (now - timers.last_physics >= 10) {
        update_physics();
        timers.last_physics = now;
    }
    
    if (now - timers.last_display >= 33) {
        update_display();
        broadcast_state();
        timers.last_display = now;
    }
    
    // ... etc
}
```

**Tarefas:**
- [ ] Implementar timer para física (10ms)
- [ ] Implementar timer para display (33ms)
- [ ] Implementar timer para lixo (10s)
- [ ] Implementar timer para reciclador (30s)
- [ ] Garantir que timers não bloqueiam

---

## 📡 Comunicação e Protocol Buffers

### Arquitetura ZMQ

**SERVER:**
- Socket **REQ/REP** - receber comandos dos clientes
- Socket **PUB** - broadcast do estado do universo

**CLIENT:**
- Socket **REQ** - enviar comandos
- Socket **SUB** - receber updates

**DASHBOARD:**
- Socket **SUB** - receber estatísticas

### Protocol Buffers - Mensagens

Criar ficheiro `.proto` com as mensagens:

```protobuf
// movement.proto
syntax = "proto3";

message ThrustCommand {
    enum Direction {
        UP = 0;
        DOWN = 1;
        LEFT = 2;
        RIGHT = 3;
    }
    Direction direction = 1;
    int32 ship_id = 2;
}

message ConnectRequest {
    string player_name = 1;
}

message ConnectResponse {
    int32 ship_id = 1;
    char planet_letter = 2;
    bool success = 3;
    string message = 4;
}

message Vector {
    float amplitude = 1;
    float angle = 2;
}

message TrashShip {
    int32 id = 1;
    float x = 2;
    float y = 3;
    Vector velocity = 4;
    int32 cargo = 5;
    int32 capacity = 6;
    char planet_letter = 7;
}

message Trash {
    int32 id = 1;
    float x = 2;
    float y = 3;
    Vector velocity = 4;
}

message Planet {
    char letter = 1;
    float x = 2;
    float y = 3;
    int32 recycled_count = 4;
    bool is_recycler = 5;
}

message UniverseState {
    repeated Planet planets = 1;
    repeated Trash trash = 2;
    repeated TrashShip ships = 3;
    int32 total_trash = 4;
    int32 max_trash = 5;
    bool collapsed = 6;
}

message DashboardStats {
    repeated Planet planets = 1;
    repeated TrashShip ships = 2;
    int32 roaming_trash = 3;
    int32 max_trash = 4;
    char recycler_planet = 5;
}
```

### ⚠️ ZMQ NÃO É THREAD-SAFE!

**REGRAS CRÍTICAS:**
- Apenas **1 thread** usa socket REQ
- Apenas **1 thread** usa socket PUB
- Nunca partilhar sockets entre threads
- Usar mutexes para dados partilhados, não para sockets

**Tarefas:**
- [ ] Desenhar arquitetura de mensagens
- [ ] Criar ficheiros .proto
- [ ] Compilar protocol buffers
- [ ] Implementar serialização/deserialização
- [ ] Testar comunicação básica
- [ ] Implementar detecção de desconexão

---

## ⚙️ Ficheiros de Configuração

Usar **libconfig++** para ler configurações.

### new-universe-server.cfg
```
# Server configuration
server:
{
    req_port = 5555;
    pub_port = 5556;
};

universe:
{
    width = 800;
    height = 600;
    num_planets = 3;
    initial_trash = 10;
    max_trash = 20;
    ship_capacity = 5;
};
```

### new-trash-ship-client.cfg
```
# Client configuration
server:
{
    address = "tcp://localhost:5555";
    sub_address = "tcp://localhost:5556";
};

universe:
{
    width = 800;
    height = 600;
};
```

### new-dashboard.cfg
```
# Dashboard configuration
server:
{
    sub_address = "tcp://localhost:5556";
};
```

**Tarefas:**
- [ ] Criar estrutura dos ficheiros .cfg
- [ ] Implementar leitura com libconfig++
- [ ] Validar configurações
- [ ] Usar configs em vez de hardcoded values

---

## 🧵 Threading e Concorrência

### Threads Necessárias no SERVER

```
Main Thread (SDL)
├── Processar eventos SDL
└── Desenhar janela

Physics Thread
├── Loop de 10ms
├── Calcular física do lixo
└── Calcular física das ships

Communication Thread
├── Receber comandos (REQ/REP)
├── Processar comandos
└── Enviar updates (PUB)

Timer Threads (ou integrados)
├── Geração de lixo (10s)
└── Mudança de reciclador (30s)
```

### Sincronização

**Dados partilhados que precisam de mutex:**
- Array de planetas
- Array de lixo
- Array de trash-ships
- Estado do universo

**Exemplo:**
```c
pthread_mutex_t universe_mutex;

// Na thread de física
pthread_mutex_lock(&universe_mutex);
update_trash_physics(trash, num_trash);
update_ships_physics(ships, num_ships);
pthread_mutex_unlock(&universe_mutex);

// Na thread de comunicação
pthread_mutex_lock(&universe_mutex);
serialize_universe_state(&state);
pthread_mutex_unlock(&universe_mutex);
zmq_send(pub_socket, &state, size, 0);
```

### Limitações do SDL

⚠️ **SDL NÃO É THREAD-SAFE:**
- Criação de janelas → **main thread**
- Desenhar na janela → **main thread**
- SDL_PollEvent / SDL_WaitEvent → **main thread**
- SDL_PushEvent → pode ser chamado de qualquer thread ✓

**Solução:** Usar SDL_PushEvent para notificar main thread de outras threads.

**Tarefas:**
- [ ] Criar threads com pthread
- [ ] Implementar mutexes para dados partilhados
- [ ] Garantir que apenas 1 thread usa cada socket
- [ ] Testar concorrência sem race conditions
- [ ] Garantir desenho apenas na main thread

---

## 📝 Ordem de Desenvolvimento Recomendada

Seguir esta ordem para melhor progressão:

1. [ ] **Renomear universe-simulator → new-universe-server**
   - Ponto de partida, reutilizar código da Parte A

2. **[ ] Geração periódica de lixo (10s)**
   - Implementar timer
   - Gerar lixo em posição aleatória

3. **[ ] Mudança periódica do planeta reciclador (30s)**
   - Timer de 30s
   - Escolha aleatória de planeta

4. **[ ] Display update rate de 30Hz (33ms)**
   - Separar física (10ms) de display (33ms)
   - Otimizar performance

5. **[ ] Renomear universe-client → new-trash-ship-client**
   - Base para o novo cliente

6. **[ ] Integrar estruturas de trash-ships no servidor**
   - Adicionar arrays e structs
   - Funções de gestão

7. **[ ] Sockets e comunicação no servidor**
   - a) **[ ]** Aceitar conexões de clientes
   - b) **[ ]** Receber e processar comandos de movimento

8. **[ ] Aplicar regras de física às trash-ships**
   - Inércia, gravidade, friction
   - Thrust como força

9. **[ ] Display do universo no cliente**
   - a) **[ ]** Criar janela SDL
   - b) **[ ]** Receber e processar estado do universo
   - c) **[ ]** Desenhar tudo (planetas, lixo, ships)

10. **[ ] Implementar new-dashboard**
    - Escolher linguagem
    - Socket subscriber
    - Display de estatísticas

11. **[ ] Todas as outras funcionalidades**
    - Colisões
    - Reciclagem
    - Validações
    - Tratamento de erros

---

## 🚫 Limites e Validações

### Limites do Universo (configuráveis)

- Número máximo de planetas
- Número máximo de trash-ships
- Capacidade das trash-ships
- Max trash antes do colapso

### Validações Necessárias

**No Servidor:**
```c
// 1. Capacidade da ship
if (ship->cargo >= ship->capacity) {
    // Não coletar mais lixo
    return;
}

// 2. Limite de conexões
if (num_connected_ships >= max_ships) {
    send_error_response("Server full");
    return;
}

// 3. Colapso do universo
if (num_trash >= max_trash) {
    collapse_universe();
    notify_all_clients();
}

// 4. Collision com planeta não-reciclador
if (collides_with_planet(ship, planet) && !planet->is_recycler) {
    spill_trash(ship);
}

// 5. Deposição no reciclador
if (collides_with_planet(ship, recycler_planet)) {
    planet->recycled_count += ship->cargo;
    ship->cargo = 0;
}
```

### Comportamentos Especiais

**Quando não há trash-ships ativos:**
- Parar geração de lixo
- Lixo que bate em planetas não gera novo lixo

**Tarefas:**
- [ ] Implementar verificação de capacidade
- [ ] Implementar limite de conexões
- [ ] Implementar detecção de colapso
- [ ] Notificar clientes apropriadamente
- [ ] Testar todos os edge cases

---

## 🔒 Anti-Cheating e Segurança

### Princípios

**Nunca confiar no cliente!**
- Cliente pode estar modificado
- Mensagens podem estar adulteradas
- Ordem pode estar errada

### Validações no Servidor

```c
// 1. Validar estrutura da mensagem
if (!validate_protobuf(message)) {
    log_error("Invalid message structure");
    return;
}

// 2. Verificar se cliente controla essa ship
if (command->ship_id != client->assigned_ship_id) {
    log_error("Client trying to control other ship!");
    return;
}

// 3. Verificar limites físicos
if (command->thrust > MAX_THRUST) {
    log_error("Thrust too high");
    command->thrust = MAX_THRUST;
}

// 4. Rate limiting
if (time_since_last_command < MIN_COMMAND_INTERVAL) {
    log_error("Commands too frequent");
    return;
}

// 5. Validar estado
if (ship->state == DISCONNECTED) {
    log_error("Ship is disconnected");
    return;
}
```

### O Que Prevenir

- ❌ Mover trash-ships de outros jogadores
- ❌ Teletransporte instantâneo
- ❌ Capacidade infinita de cargo
- ❌ Atravessar planetas sem perder cargo
- ❌ Spam de comandos
- ❌ Mensagens malformadas

### O Servidor Tem Autoridade Final

```c
// Cliente envia comando, servidor decide resultado
// NUNCA aceitar posições/velocidades diretamente do cliente
// Apenas aceitar inputs (thrust direction)
```

**Tarefas:**
- [ ] Validar estrutura das mensagens
- [ ] Verificar se cliente pode executar ação
- [ ] Prevenir movimento de ships alheias
- [ ] Rate limiting de comandos
- [ ] Testar com "cliente malicioso"

---

## 🏗️ Arquitetura de Software

### Camadas e Ficheiros

```
new-universe-server/
├── new-universe-server.c         # Main
├── display.c / display.h         # SDL drawing
├── physics-rules.c / physics-rules.h
├── universe-data.c / universe-data.h
├── communication.c / communication.h
└── Makefile

new-trash-ship-client/
├── new-trash-ship-client.c       # Main
├── display.c / display.h
├── cursor_processing.c / cursor_processing.h
├── communication.c / communication.h
└── Makefile

new-dashboard/
├── new-dashboard.py (ou .java)
└── requirements.txt (ou pom.xml)
```

### Reutilização da Parte A

Reutilizar e evoluir:
- ✓ Data structures (planetas, lixo)
- ✓ Funções de física
- ✓ Funções de desenho SDL
- ✓ Estrutura básica

Adicionar:
- ✓ Trash-ships data structures
- ✓ Threads
- ✓ ZMQ + Protocol Buffers
- ✓ Timers periódicos

---

## 🛠️ Tecnologias e Restrições

### Permitido

- ✅ SDL2 (display e input)
- ✅ libconfig++ (configurações)
- ✅ ZeroMQ TCP (comunicação)
- ✅ Protocol Buffers (encoding)
- ✅ Threads (pthread)
- ✅ Mutex/locks

### **PROIBIDO**

- ❌ select()
- ❌ Non-blocking communication
- ❌ Active wait (busy waiting)
- ❌ Signals

### Linguagens

- **C** - new-universe-server, new-trash-ship-client
- **Python ou Java** - new-dashboard

---

## 📦 Entrega e Avaliação

### Formato de Entrega

```
projeto_B_grupo_XX.zip
├── new-universe-server/
│   ├── *.c, *.h
│   ├── *.proto
│   └── Makefile
├── new-trash-ship-client/
│   ├── *.c, *.h
│   ├── *.proto
│   └── Makefile
├── new-dashboard/
│   ├── *.py (ou *.java)
│   └── dependencies
└── README.md (opcional mas recomendado)
```

### Critérios de Avaliação

- **Funcionalidades implementadas** (mais = melhor)
- **Comunicação** (Protocol Buffers + ZMQ corretos)
- **Estrutura do código** (organização, layers, abstração)
- **Tratamento de erros** (robustez)
- **Anti-cheating** (validações no servidor)
- **Comentários** (código documentado)

### Checklist Final

- [ ] Todos os 3 componentes compilam
- [ ] Makefiles funcionam
- [ ] Ficheiros de configuração funcionam
- [ ] Múltiplos clientes conectam simultaneamente
- [ ] Física funciona corretamente
- [ ] Display sincronizado entre servidor e clientes
- [ ] Dashboard exibe estatísticas
- [ ] Desconexões são tratadas
- [ ] Anti-cheating implementado
- [ ] Código comentado
- [ ] Sem memory leaks (testar com valgrind)
- [ ] Sem race conditions

---

## 📚 Recursos Úteis

### Documentação

- SDL2: https://wiki.libsdl.org/
- ZeroMQ: https://zeromq.org/
- Protocol Buffers: https://protobuf.dev/
- libconfig: documentação na pasta do projeto
- pthreads: `man pthread_create`

### Comandos Úteis

```bash
# Compilar protocol buffers
protoc --c_out=. messages.proto

# Compilar com warnings
gcc -Wall -Wextra -pthread -o server server.c

# Testar memory leaks
valgrind --leak-check=full ./server

# Testar race conditions
valgrind --tool=helgrind ./server
```

### Debugging

```bash
# Ver mensagens ZMQ
export ZMQ_VERBOSE=1

# GDB com threads
gdb ./server
(gdb) set scheduler-locking on
(gdb) info threads
(gdb) thread 2
```

---

## 💡 Dicas Finais

1. **Comece simples** - Implemente funcionalidades básicas primeiro
2. **Teste incrementalmente** - Não acumule código sem testar
3. **Use git** - Commit frequente para não perder trabalho
4. **Divida tarefas** - Trabalho em equipa eficiente
5. **Pergunte cedo** - Não deixe dúvidas para a última hora
6. **Documente** - Código comentado facilita debug
7. **Valide sempre** - Servidor nunca confia no cliente
8. **Performance** - Profile antes de otimizar

**Boa sorte! 🚀**