#ifndef __AI_PLAYER_H__
#define __AI_PLAYER_H__

# include "Attributes.h"
# include "Player.h"

class AIPlayer: public Player{
    protected:
        /**
         * Id identificativo del jugador
         */
        const int id;

    public:
        /**
         * @brief Constructor de un objeto AIPlayer
         * 
         * @param name Nombre del jugador
         */
        inline AIPlayer(const string & name):Player(name), id(0){};
        
        /**
         * @brief Constructor de un objeto AIPlayer 
         * 
         * @param name Nombre  del jugador
         * @param id Id del jugador
         */
        inline AIPlayer(const string & name, const int id):Player(name), id(id){};

        /**
         * @brief Función que percibe el el parchís y al jugador actual.
         * Asigna el tablero en actual y el id del jugador.
         * 
         * @param p Instancia Parchis que se quiere percibir
         */
        inline virtual void perceive(Parchis &p){Player::perceive(p);}

        /**
         * @brief Función abstracta que define el movimiento devuelto por el jugador.
         * Llama a la función movePiece con el valor asignado a los parámetros pasados 
         * por referencia.
         * 
         * @return true
         * @return false 
         */
        virtual bool move();
        
        /**
         * @brief Función que se encarga de decidir el mejor movimiento posible a 
         * partir del estado actual del tablero. Asigna a las variables pasadas por
         * referencia el valor de color de ficha, id de ficha y dado del mejor movimiento.
         * 
         * @param c_piece Color de la ficha
         * @param id_piece Id de la ficha
         * @param dice Número de dado
         */
        virtual void think(color & c_piece,  int & id_piece, int & dice) const;

        void thinkAleatorio(color & c_piece, int & id_piece, int & dice) const;

        void thinkAleatorioMasInteligente(color & c_piece, int & id_piece, int & dice) const;

        void thinkFichaMasAdelantada(color & c_piece, int & id_piece, int & dice) const;

        void thinkMejorOpcion(color & c_piece, int & id_piece, int & dice) const;

        /**
         * @brief Método que determina si el player es inteligente (decide el mejor movimiento)
         * o no. True para AIPlayer.
         * 
         * @return true 
         * @return false 
         */
        inline virtual bool canThink() const{return true;}

        /**
         * @brief Implementación del algoritmo MiniMax
         * @param actual
         * @param jugador
         * @param profundidad
         * @param profundidad_max
         * @param c_piece
         * @param id_piece
         * @param dice
         * @param heuristic
         * @return
         */
        double MiniMax(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, double (*heuristic)(const Parchis &, int)) const;

        /**
         * @brief Implementacioón del algoritmo Poda Alpha-Beta
         * @param actual
         * @param jugador
         * @param profundidad
         * @param profundidad_max
         * @param c_piece
         * @param id_piece
         * @param dice
         * @param alpha
         * @param beta
         * @param heuristic
         * @return
         */
        double PodaAlfaBeta(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, double alpha, double beta, double (*heuristic)(const Parchis &, int)) const;

        /**
         * @brief Heurística de prueba para validar el algoritmo de búsqueda.
         * 
         * @param estado Instancia de Parchis con el estado actual de la partida.
         * @param jugador Id del jugador actual (0 o 1)
         * @return double 
         */
        static double ValoracionTest(const Parchis &estado, int jugador);

        /**
         *
         * @param estado
         * @param jugador
         * @return
         */
        static double myHeuristic1(const Parchis &estado, int jugador);
        static double myHeuristic2(const Parchis &estado, int jugador);
        static double myHeuristic3(const Parchis &estado, int jugador);

        /**
        * @brief Calcula el ranking de la pieza especificada
        *
        * @param estado
        * @param jugador
        * @param c_piece
        * @param id_piece
        * @return
        */
        static double ranking(const Parchis &estado, int jugador, color c_piece, int id_piece);

        /**
         * @brief Comprueba si la pieza {c1,id1} puede comerse a la pieza {c2,id2}
         */
        static bool canEat(const Parchis &estado, const color c1, int id1, const color c2, int id2);

        /**
         * @brief Comprueba si la pieza {c,idx} puede llegar a la meta
         *
         * @param estado
         * @param c
         * @param idx
         * @return
         */
        static bool canReachGoal(const Parchis &estado, const color &c, int idx);

        /**
         * @brief Calcula la distancia promedio de cada pieza del color c hasta la meta. Así se tiene en cuenta
         * también la distancia entre las piezas de cada color (no es bueno dejar a las piezas rezagadas)
         *
         * @param estado
         * @return
         */
        static double getMeanDistanceToGoal(const Parchis &estado, const color c);

        /**
        * @brief Devuelve el indice de la pieza más cercana a la meta del color c (que aún no ha llegado a la meta).
        *
        * @param c
        * @return
        */
        static int getIdPieceMasAdelantada(const Parchis &estado, const color c);

};
#endif