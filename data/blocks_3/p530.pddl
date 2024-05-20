;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=530, seed=20

(define (problem blocksworld-530)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on b1 b3)
    (on b3 b5)
    (on b5 b4)
    (on-table b4))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b5)
    (on b5 b1)
    (on b1 b4)
    (on-table b4))))
