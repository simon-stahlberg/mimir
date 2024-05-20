;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=560, seed=20

(define (problem blocksworld-560)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on b5 b2)
    (on b2 b4)
    (on b4 b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b3)
    (on b3 b5)
    (on b5 b2)
    (on b2 b1)
    (on-table b1))))
