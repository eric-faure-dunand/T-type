# R-type — protocole réseau

Ce document décrit le format des messages échangés entre le client et le
serveur du jeu **R-type**.

## Principe

Avant la sérialisation, une commande peut être représentée sous la forme
suivante :

```text
MOVE 0;1 156 21;25;
```

Pour être transmise sur le réseau, cette commande est découpée en octets et
encapsulée dans une trame composée d'un en-tête et d'un message :

```text
┌──────────────┬──────────┬───────────────┬──────────────┬──────┬─────────┐
│ Magic number │ Command  │ Sequence num. │ Sequence len │ Size │ Message │
└──────────────┴──────────┴───────────────┴──────────────┴──────┴─────────┘
```

## Format d'une trame

| Champ | Taille | Description |
| --- | ---: | --- |
| **Magic number** | 2 octets | Clé connue du client et du serveur. Elle permet d'identifier les trames R-type et d'écarter les commandes externes. |
| **Command** | 1 octet | Commande à exécuter. |
| **Sequence number** | 4 octets | Numéro de la séquence à laquelle appartient le fragment. |
| **Sequence length** | 4 octets | Taille totale de la séquence. |
| **Size** | 2 octets | Taille du message, en octets. |
| **Message** | `Size` octets | Données utiles de la trame. |

Les champs sont sérialisés dans l'ordre présenté ci-dessus. La taille du champ
**Message** doit toujours correspondre à la valeur indiquée dans **Size** :
le récepteur doit lire la totalité des `Size` octets avant de traiter la
trame.

## Exemple de structure

```text
[ Magic number ]
[ Command       ]
[ Sequence num. ]
[ Sequence len  ]
[ Size          ]
[ Message       ]
```