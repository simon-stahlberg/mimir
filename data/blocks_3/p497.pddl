;; blocks=5, percentage_new_tower=0, out_folder=., instance_id=497, seed=17

(define (problem blocksworld-497)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b4)
    (on b4 b3)
    (on b3 b2)
    (on b2 b5)
    (on-table b5))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b1)
    (on b1 b5)
    (on-table b5))))
