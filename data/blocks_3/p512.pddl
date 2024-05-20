;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=512, seed=2

(define (problem blocksworld-512)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b4)
    (on-table b4)
    (clear b5)
    (on b5 b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b1)
    (on b1 b3)
    (on b3 b2)
    (on b2 b5)
    (on-table b5))))
