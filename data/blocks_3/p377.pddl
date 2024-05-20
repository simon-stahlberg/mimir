;; blocks=4, percentage_new_tower=0, out_folder=., instance_id=377, seed=17

(define (problem blocksworld-377)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))))
