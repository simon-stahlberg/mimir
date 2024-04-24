; woodworking task with 1 parts and 100% wood
; Machines:
;   1 highspeed-saw
;   1 glazer
;   1 grinder
;   1 immersion-varnisher
;   1 planer
;   1 saw
;   1 spray-varnisher

(define (problem wood-prob)
  (:domain woodworking)
  (:requirements :typing :action-costs :numeric-fluents)
  (:objects
    highspeed-saw0 - highspeed-saw
    glazer0 - glazer
    immersion-varnisher0 - immersion-varnisher
    spray-varnisher0 - spray-varnisher
    blue white - acolour
    mahogany - awood
    p0 - part
    b0 - board
    s0 s1 - aboardsize
  )
  (:init
    (grind-treatment-change varnished colourfragments)
    (grind-treatment-change glazed untreated)
    (grind-treatment-change untreated untreated)
    (grind-treatment-change colourfragments untreated)
    (is-smooth smooth)
    (is-smooth verysmooth)
    (= (total-cost) 0)
    (boardsize-successor s0 s1)
    (empty highspeed-saw0)
    (has-colour glazer0 white)
    (has-colour immersion-varnisher0 blue)
    (has-colour spray-varnisher0 white)
    (unused p0)
    (goalsize p0 small)
    (= (spray-varnish-cost p0) 5)
    (= (glaze-cost p0) 10)
    (= (grind-cost p0) 15)
    (= (plane-cost p0) 10)
    (boardsize b0 s1)
    (wood b0 mahogany)
    (surface-condition b0 smooth)
    (available b0)
  )
  (:goal
    (and
    (available p0)
    (wood p0 mahogany)
    (surface-condition p0 smooth)
    (treatment p0 varnished)
    )
  )
  (:metric minimize (total-cost))
)