#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"
#include <list>
#include <stack>

#define N 7    // macro que determina el tamaño de la matriz de potencial: debe ser IMPAR para su correcto funcionamiento
#define INF 100000000

enum estadoCasilla {
    DESCONOCIDA,
    OBSERVADA,
    VISITADA
};

struct estado {
    int fila;
    int columna;
    int orientacion;
};

class ComportamientoJugador : public Comportamiento {
    public:

        /**
        * @brief Constructor niveles 0,1,2
        * @param mapaR Copia de mapaResultado, utilizado para consulta del mapa
        */
        ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
            hayPlan = false;
            zapatillas = false;
            bikini = false;
            ultimaAccion = actIDLE;
        }

        /**
        * @brief Constructor para niveles 3,4
        * @param size Tamaño del mapa
        */
        ComportamientoJugador(unsigned int size) : Comportamiento(size) {
            zapatillas = false;
            bikini = false;
            ultimaAccion = actIDLE;
            recargando = false; // Se activará cuando se ejecute un plan de recarga
            desubicado = true;  // Inicialmente no conoce su ubicacion (en el nivel 4)
            porcentajeDescubierto = 0;
            objetivoActual.fila = -1;
            objetivoActual.columna = -1;
            indiceObjetivoActual = -1;

            // Inicializa la matriz de potencial
            potencial.clear();
            vector<unsigned int> aux1 (N, 0);
            for(int i=0; i<N; i++) potencial.push_back(aux1);

            // Inicializa la matriz de tiempo
            tiempo.clear();
            pair<estadoCasilla,unsigned int> inicial (estadoCasilla::DESCONOCIDA, 0);
            vector<pair<estadoCasilla,unsigned int>> aux2 (size, inicial);
            for(int i=0; i<size; i++) tiempo.push_back(aux2);

            colaObjetivos.empty(); // ¿Necesario?

            // Es un historial que almacena los estados más recientes. Sirve para evitar un bucle (en el caso de que el
            // jugador quede en un mismo estado (de posición) durante 4 o más iteraciones
            historial.empty();

            // Inicializa mapaResultado con los precipicios
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < size; j++) {
                    mapaResultado[i][j] = 'P';
                    mapaResultado[size-i-1][j] = 'P';
                    mapaResultado[j][i] = 'P';
                    mapaResultado[j][size-i-1] = 'P';
                }
            }
        }

        ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
        int interact(Action accion, int valor); //¿?
        ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}
        ~ComportamientoJugador(){}

        /**
        * @brief Define el comportamiento del agente
        * @param sensores Percepción sensorial actual del agente
        * @return Siguiente acción a realizar
        */
        Action think(Sensores sensores);

        /**
        * @brief Pinta sobre el mapa del juego el plan obtenido
        * @param st Estado del agente {fila, columna, orientación}
        * @param plan Plan de acciones
        */
        void VisualizaPlan(const estado &st, const list<Action> &plan);

    private:
        // ------------------------ VARIABLES DE ESTADO --------------------------------------------------
        /**
        * Variable booleana que se activa si tenemos zapatillas
        */
        bool zapatillas;

        /**
        * Variable booleana que se activa si tenemos bikini
        */
        bool bikini;

        /**
        * Variable que guarda el porcentaje del mapa descubierto
        */
        double porcentajeDescubierto;

        /**
        * Almacena la última acción realizada por el agente
        */
        Action ultimaAccion;

        /**
        * Salva la posición y orientación actual del agente
        */
        estado actual;

        /**
        * Guarda el destino actual al que ir
        */
        estado objetivoActual;

        /**
        * Almacena las coordenadas de las casillas objetivo (en función de
        * la información sensorial recibida de los sensores de num_destinos
        * y destinos)
        */
        list<estado> objetivos;

        /**
        * Variable entera que guarda el índice del objetivo actual dentro de la lista de objetivos
        */
        int indiceObjetivoActual;

        /**
        * Lista de acciones a realizar (plan)
        */
        list<Action> plan;

        /**
        * Variable booleana que se activa si hay algún plan en activo (!plan.empty())
        */
        bool hayPlan;

        /**
        * Matriz de potencial, que 'determina' el movimiento reactivo del agente
        * Tiene un tamaño reducido a N=7, ya que es lo máximo que podemos percibir
        * por los sentidos.
        */
        vector<vector<unsigned int>> potencial;

        /**
        * Matriz potencial del tiempo. Es del mismo tamaño que el mapa. Por cada casilla del mapa:
        * Si no se ha observado inicialmente tiene valor 0.
        * Si se ha observado pero no visitado se le suma cada instante de simulacion el valor 1.
        * Si se ha visitado, cada instante de simulación suma 2.
        */
        vector<vector<pair<estadoCasilla,unsigned int>>> tiempo;

        /**
        * Cola de objetivos, necesaria cuando en mitad de un plan, se percibe otro objetivo más
        * importante o prioritario.
        */
        stack<estado> colaObjetivos;

        /**
        * Variable dinaámica que almacena el estado actual. Sirve para evitar bucles, puesto que el historial
        * sólo guarda el mismo estado. Cuando el historial crezca, quiere decir que lleva muchas iteraciones
        * consecutivas en la misma casilla (estado).
        */
        list<estado> historial;

        /**
        * Bandera que se activa cuando el jugador se encuentra realizando la acción de recargar en la casilla X.
        */
        bool recargando;

        /**
        * Bandera (utilizada en el nivel 4) que se activa cuando el jugador no tiene información correcta
        * sobre su localización.
        */
        bool desubicado;

        // ------------------------ VARIABLES DE ESTADO --------------------------------------------------
        /**
        * @brief Calcula la profundidad en función a la representación del sensor de terreno
        * @param orientacion orientacion actual del jugador
        * @param i índice dentro del vector
        * @return valor positivo con la profundidad
        */
        int profundidad(int orientacion, int i);

        /**
        * @brief Calcula la lateralidad dentro del vector del sensor de terreno
        * @param orientacion orientacion actual del jugador
        * @param i indice dentro del vector de posicion
        * @return valor entero con la lateralidad (negativo si izq, positivo si dcha) en funcion del jugador
        */
        int lateralidad(int orientacion, int i);

        /**
        * @brief Actualiza el estado actual del agente. En los niveles 0-3 utiliza la información sensorial.
        * En el nivel 4 la calcula según su última acción (los sensores no funcionan).
        * @pre El agente DEBE estar ubicado (desubicado=false)
        * @post El estado actual resultante es válido y correcto.
        */
        void actualizaEstadoActual(Sensores sensores);

        /**
        * @brief (Niveles 3 y 4) Actualiza la variable mapaResultado en función de su visión y su estado
        * @param sensores Percepción sensorial del terreno en un instante
        * @pre El jugador debe estar bien posicionado antes de invocar al método
        */
        void actualizaMapaResultado(Sensores sensores);

        /**
        * @brief Actualiza acordemente el historial de estados. Es decir, si está en la misma casilla que la iteración
        * anterior, la inserta en el historial, incrementando así su tamaño en uno. Por el contrario, si se ha movido
        * de casilla (coordenadas distintas), limpia el historial (lo vacía) y lo actualiza con su estado actual. Esto
        * nos permitirá reconocer cuando el agente se ha quedado mucho tiempo en una misma casilla (cuando el historial
        * sea mayor que cierto tamaño) y así podremos evitar bucles.
        * @pre El agente debe de estar bien posicionado (desubicado=false).
        */
        void actualizaHistorialEstados();

        /**
        * @brief Actualiza la variable porcentajeDescubierto con su valor correspondiente según mapaResultado.
        */
        void actualizaPorcentajeDescubierto();

        /**
        * @brief Actualiza la memoria de tiempo donde se guardan los instantes en los que el agente pasó por
        * cada una de las casillas. Actualiza también el estado de las casillas estadoCasilla={DESCONOCIDA,OBSERVADA,VISITADA}
        * @pre El agente debe estar bien posicionado (desubicado=false).
        */
        void actualizaTiempo();

        /**
        * @brief Recalcula el potencial de la matriz en función de la información sensorial y el historial del agente.
        */
        void actualizaPotencialReactivo();

        /**
        * @brief Inicializa la variable de estado que almacena la lista de objetivos
        * @param sensores Información sensorial del agente
        * @pre La variable privada de estado list<estado> objetivos debe estar vacía (objetivos.empty())
        */
        void inicializaListaObjetivos(Sensores sensores);

        /**
        * @brief Elimina de la lista de objetivos el objetivoActual, es decir, el indicado por indiceObjetivoActual
        * @pre La variable indiceObjetivoActual debe de ser valida
        * @post Modifica la variable list<estado> objetivos
        */
        void eliminaObjetivo();

        /**
        * @brief Calcula los estados/casillas más cercanas al agente que aún no han sido exploradas.
        * @return Devuelve el estado DESCONOCIDO más cercano (según la función heurística h).
        * @pre El agente DEBE estar bien posicionado (desubicado=false).
        */
        estado obtenerObjetivoMasCercano();

        /**
        * @brief Se encarga de eliminar un plan y limpiar o invalidar las estructuras encargadas de almacenar o
        * relacionadas a dicho plan.
        * @post objetivoActual e indiceObjetivoActual quedan en un estado inválido (filas y columnas a -1)
        * @post hayPlan, recargando en false
        * @post plan.size()==0 (se vacía)
        */
        void cancelarPlan();

        /**
        * @brief Genera un plan de recarga, una vez se ha alcanzado la casilla de recarga. Principalmente,
        * calcula el número de instantes que debe pasar el agente en la casilla de recarga y genera un plan
        * con tantas acciones ociosas (actIDLE) como se consideren. La forma de calcular dichos instantes es
        * el mínimo entre el 10% de los instantes restantes o los instantes necesarios hasta recargar por completo.
        * @param sensores Información sensorial del agente para saber dónde se encuentra.
        * @return Devuelve la secuencia de acciones actIDLE para realizar.
        */
        list<Action> recargar(Sensores sensores);

        /**
        * @brief Genera una acción aleatoria distinta de actFORWARD o actWHEREIS.
        * @return Devuelve la acción aleatoria.
        */
        Action accionAleatoria();

        /**
        * @brief Un estado inválido es aquel con fila y columna fuera del rango del mapa [0, mapaResultado.size()).
        * @param st estado a comprobar
        * @return Devuelve verdadero si el estado es válido, falso en cualquier otro caso.
        */
        bool estadoValido(const estado &st);

        /**
        * @brief Comprueba si hay obstáculo justo delante (al avanzar)
        * @param st Estado: posicion y orientacion
        * @return Verdadero si no hay obstáculo (se puede avanzar)
        */
        bool HayObstaculoDelante(estado &st);

        /**
        * @brief Calcula el valor del potencial de la casilla [fila][columna] según su tipo. Cabe recalcar
        * que el potencial devuelto no tiene en cuenta el tiempo, puesto que podría no ser su potencial final
        * pues este varía según el instante de tiempo.
        * @param fila
        * @param columna
        * @return Devuelve el potencial (unsigned int) de la casilla según su tipo.
        */
        unsigned int potencialReactivo(int fila, int columna);

        /**
        * @brief Calcula y obtiene la casilla con menor potencial dentro del campo del jugador
        * @return Devuelve un estado (sin orientación) de la casilla hacia la que hay que dirigirse
        */
        estado obtenerObjetivoReactivo();

        /**
        * @brief Tiene dos políticas para establecer el objetivo deliberativo, el cual siempre va a ser una
        * casilla desconocida. En el caso normal, calcula las casillas del mapa desconocidas más cercanas al
        * agente. En el caso de que el agente haya quedado atrapado en la misma casilla más de cuatro iteraciones
        * (historial.size()>=4) entonces devuelve una casilla aleatoria desconocida dentro del mapa).
        * @return Devuelve un estado (sin orientación) de la casilla hay la que hay que dirigirse.
        * @pre El agente debe estar ubicado (desubicado==false).
        */
        estado obtenerObjetivoDeliberativo();

        /**
        * @brief Devuelve la casilla del tipo tipoCasilla más cercana al estado st (por distancia según heurística).
        * @param st
        * @param tipoCasilla
        * @return Devuelve un estado invalido si no OK
        */
        estado obtenerMasCercana(const estado &st, unsigned char tipoCasilla);

        /**
        * @brief Calcula el coste de batería relacionado a la casilla y a la siguiente accion a realizar
        * @param accion
        * @param casilla
        * @param tiene_bikini
        * @param tiene_zapatillas
        * @return Coste de batería
        */
        int c(Action accion, unsigned char casilla, bool tiene_bikini, bool tiene_zapatillas);

        /**
        * @brief Función heurística ADMISIBLE utilizada. Devuelve el máximo entre la diferencia de filas o la de columnas.
        * @param st estado origen
        * @param obj estado destino
        * @return Valor entero de la estimación de coste (distancia)
        */
        int h(const estado &st, const estado &obj);

        /**
        * @brief Posible función heurística. NO es ADMISIBLE para la implementación actual de A* puesto que
        * se permiten movimientos en diagonal.
        * Esta función NO es UTILIZADA.
        * @param st estado origen
        * @param obj estado destino
        * @return Coste estimado del camino (en función de la distancia).
        */
        int distanciaManhattan(const estado &st, const estado &obj);

        /**
        * @brief Posible función heurística. Podría ser admisible aunque no eficaz porque trabaja con números reales.
        * Esta función NO es UTILIZADA.
        * @param st estado origen
        * @param obj estado destino
        * @return Coste estimado del camino (en función de la distancia).
        */
        int distanciaEuclidea(const estado &st, const estado &obj);

        /**
        * @brief Algoritmo de búsqueda en profundidad (ya implementado, nivel 0)
        * @param origen
        * @param destino
        * @param plan
        * @return
        */
        bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);

        /**
        * @brief Algoritmo de búsqueda en anchura (para el mínimo de acciones)
        * @param origen
        * @param destino
        * @param plan
        * @return
        */
        bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);

        /**
        * @brief Algoritmo de búsqueda de costo uniforme
        * @param origen
        * @param destino
        * @param plan
        * @return
        */
        bool pathFinding_CosteUniforme(const estado &origen, const estado &destino, list<Action> &plan);

        /**
        * @brief Algoritmo de búsqueda A*
        * @param origen
        * @param destino
        * @param plan
        * @return
        */
        bool pathFinding_A(const estado &origen, const estado &destino, list<Action> &plan);

        /**
        * @brief Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
        * @param level Representa el comportamiento en el que fue iniciado el agente.
        * @param origen Casilla de origen
        * @param destino Casillas destino (caminos)
        * @param plan Lista de acciones a realizar
        * @return Devuelve verdadero si se ha podido encontrar un camino/plan
        * @pre 0<=level && level<=4
        */
        bool pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan);

        /**
        * @brief Sacar por la consola la secuencia del plan obtenido
        * @param plan Plan de acciones del agente
        */
        void PintaPlan(list<Action> plan);
};

#endif
