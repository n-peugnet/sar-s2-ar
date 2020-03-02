# TME 6

## Variables :

- `recv`
- `sent`
- `min_local`: minimum local. Initialisé avec un min aléatoire.
- `last`: contient l'id ou le numero du voisin duquel p a reçu le dernier message.


## Algo

```
R(p): {un message <x> arrive depuis q}
    Si x < min_local:
        min-local = x
    recv[q] = vrai
    last = q

D(p): {qqsoit q appartenant à Voisins(p) : recv[q]}
    décision
    pour tout r appartenant à Voisins(p) \ last:
        envoyer min_local à r

S(p): {sent = faux && q est le seul voisin dont on a pas recu de msg}
    envoyer <min_local>
    sent = vrai
```