;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=527, seed=17

(define (problem blocksworld-527)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on b2 b4)
    (on-table b4)
    (clear b3)
    (on b3 b5)
    (on b5 b1)
    (on-table b1))
 (:goal  (and
    (clear b5)
    (on b5 b4)
    (on b4 b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))
