struct test() as
    def hola() as
        print("Hola Mundo");
    end;
    def prueba() as
        println("Esto es una prueba");
        hola();
    end;
end;

let C, A;

C = test();
C.prueba();

A = 10;
println(A);

def test1(x) as
    x = 15;
end;

test1(A);
print(A);