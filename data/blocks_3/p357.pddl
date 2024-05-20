;; blocks=3, percentage_new_tower=40, out_folder=., instance_id=357, seed=27

(define (problem blocksworld-357)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2)
    (clear b3)
    (on-table b3))))
