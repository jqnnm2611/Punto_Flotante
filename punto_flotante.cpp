#include <iostream>
#include <bitset>
#include <cstdint>
#include <cmath> // esto es para que funcione NAN, INFINITY

using namespace std;

// Unión mágica para ver los bits de un float como uint32_t
union FloatUnion {
    float valor;
    uint32_t bits;
};

// Separa un float en sus partes: signo, exponente, mantisa
void descomponerFloat(float f, int& signo, int& exponente, uint32_t& mantisa) {
    FloatUnion fu;
    fu.valor = f;

    signo = (fu.bits >> 31) & 1;              // el bit más grande es el signo
    exponente = (fu.bits >> 23) & 0xFF;       // los 8 bits del medio son el exponente
    mantisa = fu.bits & 0x7FFFFF;             // los últimos 23 bits son la mantisa

    // si el número es normal (no subnormal), el 1 delante de la coma está implícito, así que lo ponemos
    if (exponente != 0)
        mantisa |= (1 << 23);
}

// Muestra los bits de un float junto con su valor
void mostrarBits(float f, string nombre) {
    FloatUnion fu;
    fu.valor = f;
    cout << nombre << ": " << bitset<32>(fu.bits) << " (" << f << ")" << endl;
}

// Esta función hace la chamba: divide manualmente dos floats como lo haría el procesador
float dividirManual(float a, float b) {
    // Casos especiales primero
    if (b == 0.0f) {
        if (a == 0.0f) return NAN;                // 0 / 0 es indefinido, así que NaN
        return (a > 0.0f) ? INFINITY : -INFINITY; // cualquier otro / 0 es infinito
    }
    if (a == 0.0f) return 0.0f;                   // 0 / algo = 0

    // Variables para almacenar todo por separado
    int sa, sb, signoR;
    int ea, eb, expR;
    uint32_t ma, mb, mantR;

    // Separamos A y B en sus partes
    descomponerFloat(a, sa, ea, ma);
    descomponerFloat(b, sb, eb, mb);

    // signo del resultado: si A y B tienen el mismo signo, es positivo, si no, negativo
    signoR = sa ^ sb;

    // restamos exponentes y le sumamos el bias (127)
    expR = (ea - eb) + 127;

    // ahora hacemos la división de las mantisas
    // usamos 64 bits para que no se pierda precisión al dividir
    uint64_t mantA = (uint64_t)ma << 23; // desplazamos para simular "coma fija"
    uint64_t mantDiv = mantA / mb;

    // toca normalizar: queremos que el bit 23 esté en 1
    if (mantDiv >= (1ULL << 24)) {
        mantDiv >>= 1;   // si hay un 1 en el bit 24, está muy grande, así que lo bajamos
        expR++;          // como lo bajamos, aumentamos el exponente
    } else {
        // si no está normalizado (el 1 no está en el bit 23), lo subimos
        while ((mantDiv & (1 << 23)) == 0 && expR > 0) {
            mantDiv <<= 1;
            expR--;
        }
    }

    // si el exponente se pasó del límite, devolvemos infinito
    if (expR >= 255)
        return (signoR == 0) ? INFINITY : -INFINITY;

    // si el exponente bajó a 0 o menos, devolvemos 0 (no manejamos subnormales aquí)
    if (expR <= 0)
        return 0.0f;

    // sacamos la mantisa final (sin el bit 1 que se asume)
    mantR = mantDiv & 0x7FFFFF;

    // armamos el float final combinando todo
    uint32_t bitsResultado = (signoR << 31) | (expR << 23) | mantR;
    FloatUnion resultado;
    resultado.bits = bitsResultado;
    return resultado.valor;
}

int main() {
    float a, b;

    cout << "Ingrese el valor de A (dividendo): ";
    cin >> a;
    cout << "Ingrese el valor de B (divisor): ";
    cin >> b;

    float resultadoManual = dividirManual(a, b);
    float resultadoCpp = a / b;

    // Mostramos los resultados con sus bits para comparar
    cout << "\n--- Resultados ---\n";
    mostrarBits(a, "A");
    mostrarBits(b, "B");
    mostrarBits(resultadoManual, "Resultado manual");
    mostrarBits(resultadoCpp, "Resultado C++");

    return 0;
}
