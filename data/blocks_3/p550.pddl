;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=550, seed=10

(define (problem blocksworld-550)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b5)
    (on b5 b4)
    (on b4 b3)
    (on-table b3)
    (clear b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b5)
    (on-table b5))))
