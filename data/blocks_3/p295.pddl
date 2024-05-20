;; blocks=3, percentage_new_tower=10, out_folder=., instance_id=295, seed=25

(define (problem blocksworld-295)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))))
