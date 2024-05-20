;; blocks=5, percentage_new_tower=0, out_folder=., instance_id=486, seed=6

(define (problem blocksworld-486)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b4)
    (on b4 b2)
    (on b2 b3)
    (on b3 b1)
    (on b1 b5)
    (on-table b5))
 (:goal  (and
    (clear b4)
    (on b4 b1)
    (on b1 b3)
    (on b3 b5)
    (on b5 b2)
    (on-table b2))))
