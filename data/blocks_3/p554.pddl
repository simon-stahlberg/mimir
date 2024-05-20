;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=554, seed=14

(define (problem blocksworld-554)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b4)
    (on-table b4)
    (clear b1)
    (on b1 b5)
    (on b5 b2)
    (on-table b2))
 (:goal  (and
    (clear b4)
    (on b4 b5)
    (on b5 b1)
    (on-table b1)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on-table b3))))
