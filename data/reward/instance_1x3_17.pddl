
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Instance file automatically generated by the Tarski FSTRIPS writer
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (problem reward-1x3)
    (:domain reward-strips)
    (:requirements :typing)
    (:objects
        c_0_0 c_0_1 c_0_2 - cell
    )

    (:init
        (adjacent c_0_1 c_0_0)
        (adjacent c_0_2 c_0_1)
        (adjacent c_0_0 c_0_1)
        (adjacent c_0_1 c_0_2)
        (at c_0_0)
        (unblocked c_0_2)
        (unblocked c_0_1)
        (unblocked c_0_0)
        (reward c_0_1)
    )

    (:goal
        (picked c_0_1)
    )




)
