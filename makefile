CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# Nombre del ejecutable
TARGET=cambios

# Archivos fuente
SRCS=cambios.c

# Biblioteca estática
LIBS=libcambios.a

# Regla para compilar el programa
$(TARGET): $(SRCS) $(LIBS)
	$(CC) -m32 $(CFLAGS) $(SRCS) $(LIBS) -o $(TARGET) 

# Regla para limpiar archivos temporales y el ejecutable
clean:
	rm -f $(TARGET)

# Indica que 'clean' no es el nombre de un archivo
.PHONY: clean
