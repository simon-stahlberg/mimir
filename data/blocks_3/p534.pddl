;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=534, seed=24

(define (problem blocksworld-534)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on b1 b5)
    (on b5 b4)
    (on-table b4))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b4)
    (on-table b4)
    (clear b3)
    (on b3 b5)
    (on b5 b1)
    (on-table b1))))
