struct test() as
    def hola() as
        print("Hola Mundo");
    end;
    def prueba() as
        println("Esto es una prueba");
        hola();
    end;
end;

C = test();
C.prueba();