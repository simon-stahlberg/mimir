;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=515, seed=5

(define (problem blocksworld-515)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on b5 b2)
    (on b2 b3)
    (on b3 b1)
    (on b1 b4)
    (on-table b4))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on b2 b5)
    (on b5 b3)
    (on b3 b4)
    (on-table b4))))
