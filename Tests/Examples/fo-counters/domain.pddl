;; Enrico Scala (enricos83@gmail.com) and Dongxu Li (dongxu.li@anu.edu.au,)
;; Reference Paper: Li, D., Scala, E., Haslum, P., & Bogomolov, S. (2018, July).
;;                  Effect-abstraction based relaxation for linear numeric planning.
;;                  In Proceedings of the 27th International
;;                  Joint Conference on Artificial Intelligence (pp. 4787-4793).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; This domain describes a set of counters that can be increased and decreased,
;; and is an extended version of counter domain.
;; The rate of such counter is however variable!

(define (domain fn-counters)
    (:requirements :strips :typing :equality :adl :numeric-fluents :action-costs)
    (:types counter)

    (:functions
        (value ?c - counter);; - int  ;; The value shown in counter ?c
        (rate_value ?c - counter);;
        (max_int);; -  int ;; The maximum integer we consider - a static value
        (total-cost)
    )

    ;; Increment the value in the given counter by one
    (:action increment
         :parameters (?c - counter)
         :precondition (and (<= (+ (value ?c) (rate_value ?c)) (max_int)))
         :effect (and (increase (value ?c) (rate_value ?c)) (increase (total-cost) 1))
    )
    ;; Decrement the value in the given counter by one
    (:action decrement
         :parameters (?c - counter)
         :precondition (and (>= (- (value ?c) (rate_value ?c)) 0))
         :effect (and (decrease (value ?c) (rate_value ?c))(increase (total-cost) 1))
    )

    (:action increase_rate
         :parameters (?c - counter)
         :precondition (and (<= (+ (rate_value ?c) 1) 10))
         :effect (and (increase (rate_value ?c) 1)(increase (total-cost) 1))
    )

    (:action decrement_rate
         :parameters (?c - counter)
         :precondition (and (>= (rate_value ?c) 1))
         :effect (and (decrease (rate_value ?c) 1)(increase (total-cost) 1))
    )


)
