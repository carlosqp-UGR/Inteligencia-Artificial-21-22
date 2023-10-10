#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <iostream>
#include "comportamientos/comportamiento.hpp"
#include <vector>
#include <queue>
using namespace std;

#define UNKNOWN '?'

class ComportamientoJugador : public Comportamiento{
public:
    /**
     * @brief Constructor de la clase
     * @param size Tamaño del mapa
     */
    ComportamientoJugador(unsigned int size) : Comportamiento(size){
        map_size = size;
        inicio_juego = true;
        init();
     }

    /**
    * @brief Constructor copia
    * @param comport
    */
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}

    /**
     * @brief Destructor
     */
    ~ComportamientoJugador(){}

    /**
    * @brief Funcion del agente reactivo
    * @param sensores Información sensorial actual
    * @return accion a realizar
    */
    Action think(Sensores sensores);

    int interact(Action accion, int valor);

private:
    const int min_bateria_para_recargar = 3000;

    // Tamaño del mapa
    unsigned int map_size;

    // Variables de estado (memoria del agente)
    int fil, col, brujula;
    Action ultimaAccion;
    bool bien_posicionado;
    bool girar_dcha;
    bool inicio_juego;

    // Salva informacion de estado
    bool tiene_bikini, tiene_zapatillas;

    int contador_forward;
    int max_forward;

    bool salida_obstaculo_izq;
    bool salida_obstaculo_dcha;

    // Memoria del agente (cuando aún no ha encontrado G)
    vector<vector<unsigned char>> mapaProvisional;

    // Parejas de valores, almacenan la minima posicion y la maxima posicion descubierta en mapaProvisional
    pair<int,int> min_coordenadas;  // fila, col
    pair<int,int> max_coordenadas;

    queue<Action> accionCompuesta;
    int objetivo; // Casilla objetivo

    bool recargar_bateria;

// Métodos adicionales
    /**
     * @brief Restablece las variables de estado a su valor por defecto
     */
    void init();

    /**
     * @brief Reestablece los valores por defecto tras morir
     */
    void reestablecer();

    /**
     * @brief Devuelve true si la casilla objetivo se percibe por los sensores
     * @param obj Caracter tipo de la casilla
     * @param sensores
     * @return true si se encuentra en la vision
     */
     int en_vision(unsigned char obj, Sensores sensores);

     /**
      * @brief Genera una secuencia de acciones para llegar a un objetivo
      * @param obj índice de la casilla de visión a la que se desea llegar
      * @post Actualiza la cola accionCompuesta
      */
     void perseguir(int obj);

// Métodos para percepción y posicionamiento
    /**
     * @brief Calcula la profundidad en función a la representación del sensor de terreno o de superficie
     * @param i índice dentro del vector
     * @return valor positivo con la profundidad
     */
    int profundidad(int i);

    /**
     * @brief Calcula la lateralidad dentro del vector del sensor de terreno/superficie
     * @param i indice dentro del vector de posicion
     * @return valor entero con la lateralidad (negativo si izq, positivo si dcha) en funcion del centro 0
     */
    int lateralidad(int i);

    /**
     * @brief actualiza la variable mapaResultado en función de su visión (sensor
     * @pre el jugador debe estar bien posicionado antes de invocar al método
     */
    void actualizaMapaResultado(Sensores sensores);

    /**
     * @brief Actualiza el mapa provisional con información actualizada de sensores
     * @param sensores
     */
    void actualizaMapaProvisional(Sensores sensores);

// Métodos auxiliares para actualizar el mapa de memoria auxiliar y
// volcarlo en la memoria real
    /**
     * @brief Calcula el ángulo de giro correspondiente (orientacion_real - brujula_relativa)
     * @param orientacion_real
     * @return Grados de giro en sentido horario
     */
    int rotacion (int orientacion_real);

    /**
     * @brief Genera una submatriz de dimensiones reducidas, ajustando mapaProvisional
     * en función del área recorrida (min_coordenadas y max_coordenadas)
     * @post Modifica mapaProvisional
     * @post Modifica las variables fil, col
     */
    void submatriz();

    /**
     * @brief Convierte mapaProvisional en su matriz traspuesta
     * @post Modifica mapaProvisional
     * @post Modifica las variables fil, col
     */
    void traspuesta();

    /**
     * @brief Rota 90 grados en sentido horario la matriz mapaProvisional
     * @post Modifica mapaProvisional
     * @post Modifica las variables fil, col
     */
    void rotar_90_grados();

    /**
     * @brief Rota 180 grados en sentido horario la matriz mapaProvisional.
     * @post Modifica mapaProvisional
     * @post Modifica las variables fil, col
     */
    void rotar_180_grados();

    /**
     * @brief Rota 270 grados en sentido horario la matriz mapaProvisional
     * @post Modifica mapaProvisional
     * @post Modifica las variables fil, col
     */
    void rotar_270_grados();

    /**
     * @brief Vuelca la información recopilada en mapaProvisional en mapaResultado
     * realizando las transformaciones necesarias para la vericidad de esta.
     * Posteriormente limpia mapaProvisional (ya no es necesaria).
     * @param sensores
     * @pre Se debe de tener información correcta en los sensores, es decir, se debe
     * de estar ubicado en la casilla de posicionamiento G
     */
    void transforma_matriz(Sensores sensores);

/*
    // Método para depurar
    template <typename T>
    void imprime(vector<vector<T>> m, Sensores s) {
        if (bien_posicionado) cout << "bien_posicionado = true" << endl;
        else cout << "bien_posicionado = false" << endl;

        cout << "Posicion_Relativa = [" << fil << "," << col << "]" << endl;
        cout << "Posicion_Real =\t[" << s.posF << "," << s.posC << "]" << endl;

        for (int i = 0; i < m.size(); ++i) {
            for (int j = 0; j < m.at(i).size(); ++j) {
                if (fil == i && col == j) cout << "X ";
                else if (s.posF == i && s.posC == j) cout << "R ";
                else cout << m[i][j] << " ";
            }
            cout << endl;
        }
    }
*/

};

#endif