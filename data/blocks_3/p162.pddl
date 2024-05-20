;; blocks=2, percentage_new_tower=10, out_folder=., instance_id=162, seed=12

(define (problem blocksworld-162)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2))))
