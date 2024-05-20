;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=255, seed=15

(define (problem blocksworld-255)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on-table b2))))
