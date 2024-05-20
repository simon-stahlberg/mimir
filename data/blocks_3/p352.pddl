;; blocks=3, percentage_new_tower=40, out_folder=., instance_id=352, seed=22

(define (problem blocksworld-352)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on b2 b3)
    (on-table b3)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on-table b3)
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2))))
