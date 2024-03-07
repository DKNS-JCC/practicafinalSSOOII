CC=gcc
CFLAGS=-Wall -Wextra -std=c99
LDFLAGS=-lm

# Nombre del ejecutable
TARGET=cambios

# Archivos fuente
SRCS=cambios.c

# Biblioteca estática
LIBS=libcambios.a

# Regla para compilar el programa
$(TARGET): $(SRCS) $(LIBS)
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -o $(TARGET) $(LDFLAGS)

# Regla para limpiar archivos temporales y el ejecutable
clean:
	rm -f $(TARGET)

# Indica que 'clean' no es el nombre de un archivo
.PHONY: clean
