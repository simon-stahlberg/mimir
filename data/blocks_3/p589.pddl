;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=589, seed=19

(define (problem blocksworld-589)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2)
    (clear b5)
    (on-table b5)
    (clear b4)
    (on-table b4))
 (:goal  (and
    (clear b4)
    (on b4 b5)
    (on-table b5)
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on-table b3))))
