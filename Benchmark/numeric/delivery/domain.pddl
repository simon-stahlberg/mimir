;; Alexander Shleyfman (shleyfman.alexander@gmail.com) and Ryo Kuroiwa (mhgeoe@gmail.com)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This is a simple numeric domain where robots
; with multiple arms and a tray deliver some items
; on a map represented via a directed graph.
; The items have numeric weights, and the robots have weight capacity limits

(define (domain delivery)
    (:requirements :typing :numeric-fluents)
    (:types
        room item arm bot - object
    )
    (:predicates
        (at-bot ?b - bot ?x - room)
        (at ?i - item ?x - room)
        (door ?x - room ?y - room)
        (free ?a - arm)
        (in-arm ?i - item ?a - arm)
        (in-tray ?i - item ?b - bot)
        (mount ?a - arm ?b - bot)
    )

    (:functions
        (load_limit ?b - bot)
        (current_load ?b - bot)
        (weight ?i - item)
        (cost)
    )

    (:action move
        :parameters (?b - bot ?x - room ?y - room)
        :precondition (and (at-bot ?b ?x)
            (door ?x ?y))
        :effect (and (at-bot ?b ?y)
            (not (at-bot ?b ?x))
            (increase (cost) 3)
        )
    )

    (:action pick
        :parameters (?i - item ?x - room ?a - arm ?b - bot)
        :precondition (and (at ?i ?x) (at-bot ?b ?x) (free ?a) (mount ?a ?b)
            (<= (+ (current_load ?b) (weight ?i)) (load_limit ?b)))
        :effect (and (in-arm ?i ?a)
            (not (at ?i ?x))
            (not (free ?a))
            (increase (current_load ?b) (weight ?i))
            (increase (cost) 2)
        )
    )

    (:action drop
        :parameters (?i - item ?x - room ?a - arm ?b - bot)
        :precondition (and (in-arm ?i ?a)
            (at-bot ?b ?x)
            (mount ?a ?b))
        :effect (and (free ?a)
            (at ?i ?x)
            (not (in-arm ?i ?a))
            (decrease (current_load ?b) (weight ?i))
            (increase (cost) 2)
        )
    )

    (:action to-tray
        :parameters (?i - item ?a - arm ?b - bot)
        :precondition (and (in-arm ?i ?a)
            (mount ?a ?b))
        :effect (and (free ?a)
            (not (in-arm ?i ?a))
            (in-tray ?i ?b)
            (increase (cost) 1)
        )
    )

    (:action from-tray
        :parameters (?i - item ?a - arm ?b - bot)
        :precondition (and (in-tray ?i ?b)
            (mount ?a ?b)
            (free ?a))
        :effect (and (not (free ?a))
            (in-arm ?i ?a)
            (not (in-tray ?i ?b))
            (increase (cost) 1)
        )
    )
)
