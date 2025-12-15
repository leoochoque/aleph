struct test() as
    def hola() as
        println("Hola Mundo");
    end;
    def prueba() as
        println("Esto es una prueba");
        hola();
    end;
end;

C = test();
C.prueba();

test = 10;
struct Contador(inicial) as
    val = inicial;
    def incrementar() as
        test = 30;
        self.val = val + 1;
        return self.val;
    end;

    def obtener() as
        return self.tranca;
    end;

    tranca = 15;
end;

c = Contador(5);
println(c.incrementar()); # => 6
println(c.incrementar()); # => 7
println(c.obtener());     # => 7
println(test); # => 10

struct Contador2(ini) as
    val = ini;

    def incrementar() as
        self.val = val + 1;
        return self; # Devolvemos el objeto mismo
    end;

    def imprimir() as
        print(val);
        return self;
    end;
end;

# Uso normal
c = Contador2(0);

# Uso con Self (Encadenamiento)
c.incrementar().incrementar().imprimir().incrementar(); 

# Salida: 2
# (Y el objeto queda en 3 internamente)

println ""; # Nueva línea para claridad
println "";

struct Persona(nombre) as
    def cambiarNombre(nombre) as
        self.nombre = nombre; 
    end;
end;

p = Persona("Juan");
println(p.nombre); # Imprime "Juan"
p.cambiarNombre("Pedro");
println(p.nombre); # Imprime "Pedro"

