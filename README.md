# CA_GoL
Cellular Automata - Conway's game of life, written in C using SDL2 for graphics. 

Follows standard GoL rules, with an addition of state which only affects rendered color: 

States: 
1. Stasis
2. Born
3. Death
4. Nothingness


# Compile with:
```
gcc casdl.c -lSDL2main -lSDL2 -Wall -Wextra -o gol
``` 