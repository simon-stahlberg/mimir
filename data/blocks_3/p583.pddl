;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=583, seed=13

(define (problem blocksworld-583)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b4)
    (on b4 b2)
    (on-table b2)
    (clear b3)
    (on-table b3)
    (clear b5)
    (on-table b5))
 (:goal  (and
    (clear b3)
    (on b3 b1)
    (on-table b1)
    (clear b4)
    (on b4 b5)
    (on b5 b2)
    (on-table b2))))
