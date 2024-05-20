;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=546, seed=6

(define (problem blocksworld-546)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on b2 b5)
    (on b5 b1)
    (on b1 b3)
    (on b3 b4)
    (on-table b4))
 (:goal  (and
    (clear b5)
    (on b5 b1)
    (on b1 b3)
    (on-table b3)
    (clear b4)
    (on b4 b2)
    (on-table b2))))
