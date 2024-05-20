;; blocks=2, percentage_new_tower=0, out_folder=., instance_id=134, seed=14

(define (problem blocksworld-134)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on-table b1))))
