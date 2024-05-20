;; blocks=5, percentage_new_tower=0, out_folder=., instance_id=509, seed=29

(define (problem blocksworld-509)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b5)
    (on b5 b1)
    (on b1 b4)
    (on-table b4))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b5)
    (on b5 b1)
    (on-table b1))))
