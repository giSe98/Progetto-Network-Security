# Progetto-Network-Security
**Traccia**: L’obiettivo del seguente progetto è l’implementazione di un insieme di attacchi, e delle rispettive mitigazioni, in un ambiente IoT simulato. In questo tipo di contesti abbiamo a che fare con dispositivi dotati di bassa potenza di elaborazione che comunicano, ed è proprio a causa di tali caratteristiche che, i protocolli di comunicazioni devono essere leggeri in modo da poter essere eseguiti senza causare un overhead eccessivo. Per tale motivo spesso, tali protocolli, tralasciano alcuni aspetti di sicurezza che richiederebbero un elevato quantitativo di risorse, lasciando uno spazio per potenziali attacchi. Nel nostro caso di studio specifico sfrutteremo questi spiragli di sicurezza in riferimento ai protocolli *6LoWPAN* e *RPL*, implementando in particolare i seguenti attacchi:
* DoS Attack
* Rank Attack
* Wormhole Attack
* Fragment Duplication Attack

**Professore**: _De Rango Floriano_

**Candidati**:
* Danny Di Cello 
* Raffaele Miriello 
* Giuseppe Seminara 
* Roberto Magurno 

## Installazione e configurazione dell'ambiente
Sebbene sia possibile seguire diverse strade per l’installazione del sistema *Contiki*, si è deciso di seguire l’approccio consigliato nella documentazione ufficiale, utilizzando quindi un’immagine Docker. La scelta effettuata presenta diversi vantaggi, i cui due più rilevanti sono i seguenti:
* **Facilità di installazione**: l’installazione richiede pochi passi al termine dei quali si dispone di un ambiente totalmente configurato e funzionante.
* **Compatibilità**: l’immagine Docker fornisce una maggiore compatibilità ed un maggior numero di dispositivi simulati, rispetto ad un processo di installazione nativa sul sistema Host.

### Requisiti
Se non è già presente sul sistema è necessario installare il pacchetto *Docker-CE*, con il seguente comando:
```shell
$ sudo apt-get install docker-ce
``` 
Successivamente è opportuno aggiungere l’utente corrente al gruppo Docker. Questo passaggio è facoltativo, ma permette di interagire col Docker senza l’utilizzo dei privilegi di amministratore.
```shell
$ sudo usermod -aG docker <nome_utente>
``` 

### Installazione 
Installato *Docker-CE* sul sistema host è possibile procedere al download dell’immagine di *Contiki-ng* digitando
```shell
$ docker pull contiker/contiki-ng
```
Tale comando scarica l’ultima versione di Contiki disponibile. Una volta completato il download è necessario clonare il repository GitHub di *Contiki-ng* e inizializzare tutti i sotto moduli con i seguenti comandi:
```shell
$ git clone https://github.com/contiki-ng/contiki-ng.git
$ cd contiki-ng
$ git submodule update --init –recursive
```
Infine, per semplificare il processo di avvio è stato creato il seguente alias:
```shell
export CNG_PATH=/home/ns/contiki-ng alias contiker="docker run –privileged–sysctl net.ipv6.conf.all.disable_ipv6=0 –mount type=bind,source=$CNG_PATH, destination=/home/user/contiki-ng -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/bus/usb:/dev/bus/usb -ti contiker/contiki-ng"
```
A questo punto l’ambiente di sviluppo è installato ed è possibile lanciare il simulatore digitando il comando 
```shell
$ contiker cooja
```

### Struttura Cartelle
Per la riproduzione degli esperimenti sviluppati è necessario identificare due cartelle importanti:
* *~/contiki-ng*: contiene tutti i file di configurazione di Contiki-NG;
* *~/contiki-ng/<nome_attacco>*: contiene tutti i file modificati per implementare gli attacchi e le rispettive mitigazioni.
La cartella attacchi è suddivisa in sottocartelle ognuna delle quali contiene l’implementazione di un attacco e della rispettiva mitigazione, riferita a topologie differenti. Inoltre, è presente uno script Bash che permette di preparare tutti i file necessari e pulire l’ambiente prima di caricare una simulazione, utilizzando i file di backup presenti nella rispettiva cartella.

