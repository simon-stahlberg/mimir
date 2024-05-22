;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=415, seed=25

(define (problem blocksworld-415)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b2)
    (on b2 b4)
    (on b4 b3)
    (on-table b3)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))