;; Enrico Scala (enricos83@gmail.com) and Miquel Ramirez (miquel.ramirez@gmail.com)
;; Reference paper: Scala, Enrico, Patrik Haslum, Sylvie Thiébaux, and Miquel Ramirez.
;;                  "Subgoaling techniques for satisficing and optimal numeric planning."
;;                  Journal of Artificial Intelligence Research 68 (2020): 691-752.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; The Farmland domain models allocating manpower to farms, with a
;; hard constraint on a metric measuring benefit. The contribution of
;; each farm to the benefit is a function of the number of workers assigned
;; Each farm requires at least one worker
(define (domain farmland)
    (:requirements :typing :numeric-fluents :negative-preconditions :equality)
    (:types
        farm - object
    )
    (:predicates
        (adj ?f1 ?f2 - farm)
    )
    (:functions
        (x ?b - farm)
        (cost)
    )

    ;; Move a person from a unit f1 to a unit f2
    (:action move-fast
        :parameters (?f1 ?f2 - farm)
        :precondition (and (not (= ?f1 ?f2)) (>= (x ?f1) 4) (adj ?f1 ?f2))
        :effect (and(decrease (x ?f1) 4) (increase (x ?f2) 2) (increase (cost) 1))
    )

    (:action move-slow
        :parameters (?f1 ?f2 - farm)
        :precondition (and (not (= ?f1 ?f2)) (>= (x ?f1) 1) (adj ?f1 ?f2))
        :effect (and(decrease (x ?f1) 1) (increase (x ?f2) 1))
    )
)
