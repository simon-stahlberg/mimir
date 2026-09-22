;; Enrico Scala (enricos83@gmail.com) and Miquel Ramirez (miquel.ramirez@gmail.com)
; Reference Paper: Scala, Enrico, Patrik Haslum, Sylvie Thiébaux, and Miquel Ramirez.
;                  "Subgoaling techniques for satisficing and optimal numeric planning."
;                  Journal of Artificial Intelligence Research 68 (2020): 691-752.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; The Sailing domain models a number of sailing boats
; whose task is to rescue people in an unbounded area of the ocean.
; The positions of the boat and people to be rescued are described by their 2D coordinates.
; The model tries to take into account the different speeds that can be obtained in a sailing boat.
; Going upwind or fully downwind is slower. We assume that wind comes from the north.

(define (domain sailing)
     (:requirements :typing :numeric-fluents)
     (:types
          boat - object
          person - object
     )
     (:predicates
          (saved ?t - person)
     )
     (:functions
          (x ?b - boat)
          (y ?b - boat)
          (d ?t - person)
     )
     ;; Increment the value in the given counter by one
     (:action go_north_east
          :parameters (?b - boat)

          :effect (and(increase (x ?b) 1.5) (increase (y ?b) 1.5))
     )
     (:action go_north_west
          :parameters (?b - boat)

          :effect (and(decrease (x ?b) 1.5) (increase (y ?b) 1.5))
     )
     (:action go_est
          :parameters (?b - boat)

          :effect (and(increase (x ?b) 3))
     )
     (:action go_west
          :parameters (?b - boat)

          :effect (and(decrease (x ?b) 3))
     )
     (:action go_south_west
          :parameters(?b - boat)

          :effect (and(increase (x ?b) 2) (decrease (y ?b) 2))
     )
     (:action go_south_east
          :parameters(?b - boat)

          :effect (and(decrease (x ?b) 2) (decrease (y ?b) 2))
     )
     (:action go_south
          :parameters(?b - boat)

          :effect (and (decrease (y ?b) 2))
     )
     (:action save_person
          :parameters(?b - boat ?t - person)
          :precondition ( and (>= (+ (x ?b) (y ?b)) (d ?t))
               (>= (- (y ?b) (x ?b)) (d ?t))
               (<= (+ (x ?b) (y ?b)) (+ (d ?t) 25))
               (<= (- (y ?b) (x ?b)) (+ (d ?t) 25))
          )
          :effect (and(saved ?t))
     )

)
