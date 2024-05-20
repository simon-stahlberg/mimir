;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=411, seed=21

(define (problem blocksworld-411)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2)
    (clear b3)
    (on b3 b4)
    (on-table b4))))
