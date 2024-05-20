;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=312, seed=12

(define (problem blocksworld-312)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b3)
    (on b3 b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on-table b3))))
