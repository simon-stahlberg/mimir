;; blocks=4, percentage_new_tower=0, out_folder=., instance_id=389, seed=29

(define (problem blocksworld-389)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b4)
    (on b4 b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b4)
    (on b4 b2)
    (on-table b2))))
