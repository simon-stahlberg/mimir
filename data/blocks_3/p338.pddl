;; blocks=3, percentage_new_tower=40, out_folder=., instance_id=338, seed=8

(define (problem blocksworld-338)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on-table b3)
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on-table b1)
    (clear b2)
    (on b2 b3)
    (on-table b3))))
