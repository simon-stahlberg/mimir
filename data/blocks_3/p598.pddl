;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=598, seed=28

(define (problem blocksworld-598)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b5)
    (on b5 b4)
    (on b4 b1)
    (on b1 b3)
    (on-table b3))
 (:goal  (and
    (clear b5)
    (on b5 b4)
    (on b4 b2)
    (on-table b2)
    (clear b1)
    (on-table b1)
    (clear b3)
    (on-table b3))))
