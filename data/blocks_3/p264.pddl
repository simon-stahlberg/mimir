;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=264, seed=24

(define (problem blocksworld-264)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))))
