#include <iostream>
#include <bitset>
#include <cstdint>
#include <cmath>

using namespace std;

//Función unión sirve para ver los bits de un float como uint32_t
union FloatUnion {
    float valor;
    uint32_t bits;
};

//Separa un float en sus partes: signo, exponente, mantisa
void descomponerFloat(float f, int& signo, int& exponente, uint32_t& mantisa) {
    FloatUnion fu;
    fu.valor = f;

    signo = (fu.bits >> 31) & 1;              //El bit más grande es el signo
    exponente = (fu.bits >> 23) & 0xFF;       //Los 8 bits del medio son el exponente
    mantisa = fu.bits & 0x7FFFFF;             //Los últimos 23 bits son la mantisa

    //Si el número es normal, el 1 delante de la coma está implícito
    if (exponente != 0)
        mantisa |= (1 << 23);
}

//Muestra los bits de un float junto con su valor
void mostrarBits(float f, string nombre) {
    FloatUnion fu;
    fu.valor = f;
    cout << nombre << ": " << bitset<32>(fu.bits) << " (" << f << ")" << endl;
}

//Divide manualmente dos floats
float dividirManual(float a, float b) {
    // Casos especiales primero
    if (b == 0.0f) {
        if (a == 0.0f) return NAN;                // 0 / 0 es indefinido, así que NaN
        return (a > 0.0f) ? INFINITY : -INFINITY; // cualquier otro / 0 es infinito
    }
    if (a == 0.0f) return 0.0f;                   // 0 / algo = 0

    //Variables para almacenar todo por separado
    int sa, sb, signoR;
    int ea, eb, expR;
    uint32_t ma, mb, mantR;

    //Separar A y B con sus respectivas partes
    descomponerFloat(a, sa, ea, ma);
    descomponerFloat(b, sb, eb, mb);

    //Si A y B tienen el mismo signo, es positivo, si no, negativo
    signoR = sa ^ sb;

    //Restar exponentes y sumar el bias
    expR = (ea - eb) + 127;

    //Ahora se debe dividir las mantisas
    //Se usan 64 bits para que la división no tenga errores
    uint64_t mantA = (uint64_t)ma << 23; //Desplazamos para simular la "coma fija"
    uint64_t mantDiv = mantA / mb;

    //Esta condicional sirve para que el bit 23 esté en 1
    if (mantDiv >= (1ULL << 24)) {
        mantDiv >>= 1;   //Si hay un 1 en el bit 24, se debe bajar
        expR++;          //Luego de bajarlo, se aumenta el exponente
    } else {
        //Si no está normalizado (el 1 no está en el bit 23), se sube
        while ((mantDiv & (1 << 23)) == 0 && expR > 0) {
            mantDiv <<= 1;
            expR--;
        }
    }

    //Si el exponente se pasó del límite, se debe devolver infinito
    if (expR >= 255)
        return (signoR == 0) ? INFINITY : -INFINITY;

    //Si el exponente bajó a 0 o menos, devolver 0
    if (expR <= 0)
        return 0.0f;

    //Sacar la mantisa final sin el bit 1 que se asume
    mantR = mantDiv & 0x7FFFFF;

    //Armar el float final combinando todo lo procesado anteriormente
    uint32_t bitsResultado = (signoR << 31) | (expR << 23) | mantR;
    FloatUnion resultado;
    resultado.bits = bitsResultado;
    return resultado.valor;
}

int main() {
    float a, b;

    cout << "Ingrese un número: ";
    cin >> a;
    cout << "Ingrese otro número que trabajará como divisor: ";
    cin >> b;

    float resultadoManual = dividirManual(a, b);
    float resultadoCpp = a / b;

    cout << "\n--- Resultados ---\n";
    mostrarBits(a, "A");
    mostrarBits(b, "B");
    mostrarBits(resultadoManual, "Resultado manual");
    mostrarBits(resultadoCpp, "Resultado C++");

    return 0;
}
