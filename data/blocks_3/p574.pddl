;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=574, seed=4

(define (problem blocksworld-574)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on-table b5)
    (clear b3)
    (on b3 b2)
    (on-table b2)
    (clear b4)
    (on b4 b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b5)
    (on b5 b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))
