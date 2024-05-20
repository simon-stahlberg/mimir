;; blocks=4, percentage_new_tower=20, out_folder=., instance_id=429, seed=9

(define (problem blocksworld-429)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on-table b3))
 (:goal  (and
    (clear b4)
    (on b4 b2)
    (on-table b2)
    (clear b1)
    (on b1 b3)
    (on-table b3))))
