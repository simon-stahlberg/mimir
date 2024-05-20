;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=321, seed=21

(define (problem blocksworld-321)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on b1 b3)
    (on-table b3))))
