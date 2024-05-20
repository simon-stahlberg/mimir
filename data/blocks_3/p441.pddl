;; blocks=4, percentage_new_tower=20, out_folder=., instance_id=441, seed=21

(define (problem blocksworld-441)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on b1 b4)
    (on b4 b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b3)
    (on-table b3)
    (clear b4)
    (on b4 b2)
    (on b2 b1)
    (on-table b1))))
