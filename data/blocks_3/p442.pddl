;; blocks=4, percentage_new_tower=20, out_folder=., instance_id=442, seed=22

(define (problem blocksworld-442)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on b1 b4)
    (on-table b4))
 (:goal  (and
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on b2 b4)
    (on-table b4))))
