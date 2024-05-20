;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=517, seed=7

(define (problem blocksworld-517)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b4)
    (on b4 b3)
    (on b3 b5)
    (on b5 b2)
    (on-table b2))
 (:goal  (and
    (clear b4)
    (on b4 b5)
    (on b5 b2)
    (on b2 b1)
    (on b1 b3)
    (on-table b3))))
