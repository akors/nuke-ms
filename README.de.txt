Dies ist die Readme-Datei für das Nuclear Messaging System Projekt. In dieser Datei finden Sie Information zum Projekt und darüber wie Sie nuke-ms bekommen und benutzen können.

Das Nuclear Messaging System (nuke-ms) ist gedacht als ein verlässliches und sicheres verteiltes Sofortnachrichtensystem, das auf vielen Plattformen verfügbar ist.
Zurzeit bietet es nur sehr rudimentäre Funktionen und ist wahrscheinlich nur zum Testen und Evaluieren zu gebrauchen.

nuke-ms ist ein freies Softwareprojekt geschrieben von Alexander Korsunsky. Sie können es unter den Bedingungen der GNU General Public License version 3 (GPLv3) verwenden , verändern und weitergeben. Diese Bedingungen können Sie in der Datei LICENSE.txt im "doc"-Verzeichnis nachsehen.

Das Projekt ist auf BerliOS Developer, eine deutsche Organisation zur Unterstützung der Entwicklung von Open Source Projekten, gehostet. Verschiedene Dienste wie zum Beispiel ein Git Repository, ein Bugtracker oder ein Wiki werden zur Verfügung gestellt. Sie können sich die nuke-ms Projektseite hier ansehen:  http://developer.berlios.de/projects/nuke-ms/


================================================================================
Inhalt dieser Datei:

1. Woher man nuke-ms bekommen kann
    1.1 Git Repository
    1.2 Herunterladen des letzten Releases von der BerliOS Projektseite

2. Installation

3. Benutzung

4. Anmerkungen zur Kompatibilität

5. Hilfe, Kommentare, Fehlermeldungen und Patches

---------------------------------------------------------------------------------

1) Woher man nuke-ms bekommen kann

1.1) Git Repository

Sie können nuke-ms aus dem Git Versinskontrollsystem auf BerliOS herunterladen.
Dazu installieren Sie Git (anweisungen können Sie hier: http://git.or.cz/gitwiki/Installation oder hier: http://progit.org/book/de/ch1-7.html finden) und wechseln Sie in das Verzeichnis in das Sie den Projektordner ablegen wollen. Sie können das Repository auschecken indem Sie folgenden Befehl in die Kommandozeile eingeben:

    git clone git://git.berlios.de/nuke-ms

Dieser Befehl erstellt einen Ordner mit dem Namen "nuke-ms" mit der neuesten Version aller Projektdateien. Um mit der Entwicklung von nuke-ms Schritt zu halten, können Sie die neuesten Änderung herunterladen indem Sie ins Projektverzeichnis wechseln und aus dem Repository "ziehen":

    $ cd nuke-ms
    $ git pull

1.2) Herunterladen des letzten releases von der BerliOS Projektseite

Sie können den Quellcode und eine für Windows kompilierte Version von nuke-ms von der BerliOS Projektseite unter folgenden Addressen herunterladen:

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.2-src.tar.gz
        Die ist die Quellcodeversion von nuke-ms mit Unix Zeilenenden (LF).

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.2-win32.zip
        Dies ist die kompilierte Win32 Version, alle Dateien haben Windows Zeilenenden (CR LF).


---------------------------------------------------------------------------------

2) Installation

Für die kompilierte Win32 Version, laden Sie das Archiv aus dem Link von oben herunter und entpacken Sie es. Die ausführbaren Dateien können Sie im "bin"-Verzeichnis des Projektordners finden, sie heißen "nuke-ms-client.exe" und "nuke-ms-serv.exe". Einige DLL werden von nuke-ms benötigt, diese liegen im bin/ Verzeichnis bei. Stellen Sie sicher dass die DLL-Datei im gleichen Verzeichnis wie die EXE-Dateien liegt.

Für die Quellcodeversion sehen die Kurzanweisungen zum Kompilieren unter unixähnlichen Systemen wie folgt aus:
    - Installieren Sie CMake >= 2.6, wxWidgets >= 2.8 and Boost >= 1.39
    - Wechseln Sie in ein Verzeichnis wo Sie nuke-ms bauen wollen, starten Sie "cmake" mit dem Verzeichnis des Projektordners und starten Sie schließlich "make":
        $ cd nuke-ms/build
        $ cmake ../src
        $ make
    Die ausführbaren Dateien können Sie im "bin"-Verzeichnis des Verzeichnisses wo Sie nuke-ms gebaut haben finden.

Für detaillierte Anweisungen zur Installation der Voraussetzungen und zum Kompilieren auf unixähnlichen- oder Windowssystemen, sehen Sie in der Datei "INSTALL.de.txt" im "doc"-Verzeichnis des Projektordners nach.


---------------------------------------------------------------------------------

3. Benutzung

nuke.ms besteht aus zwei Programmen: einem Client der Nachrichten an den Server sended und von dort empfängt und "nuke-ms-client" client heißt und einem einfachen Server, der die Nachrichten von Clients empfängt und sie weiterleitet der "nuke-ms-serv" genannt wird.


Starten Sie den Server indem Sie einfach die Datei nuke-ms-serv ausführen. Der Server übernimmt keine Parameter, zeigt weder eine grafische- noch eine kommandozeilenumgebung sondern lauscht auf dem Port 34443 auf eingehende Verbindungen. Wenn Sie eine "nörgelnde" Firewall haben, müssen Sie dem Server das Binden an den Port erlauben, also auf den Button "Erlauben", "Nicht blocken", "Entblocken" oder etwas ähnliches im Firewallfenster klicken.
Um den Server zu stoppen müssen Sie ihn von außen unterbrechen, das heißt entweder dadurch dass Sie Strg-C im Konsolenfenster angeben oder die Anwendung mit dem "kill"-Programm oder mit dem Task Manager beenden.

Starten Sie dann die nuke-ms-client Anwendung. Es sollte nun ein Fenster mit zwei Textfeldern zu sehen sein. Verbinden Sie sich mit einem Server, indem Sie ins untere Textfeld folgendes eingeben:
    /connect <host> <port>
Ersetzen Sie <host> mit dem Hostnamen des Zielrechners (IP-Addresse oder DNS Name). Wenn das Ihr lokaler Rechner ist, geben Sie "localhost" ein.
Der <port> ist immer 34443. Das erste Zeichen des kommandos muss '/' (ein Slash) sein, ansonsten wird das kommando nicht angenommen.
Wenn Sie eine Antwort so ähnlich wie "Connection succeeded." sehen, können Sie nun anfangen Nachrichten einzutippen. Sobald Sie die Entertaste gedrückt haben wid die Nachricht abgeschickt. Sie können einen Zeileinschub mit der Tastenkombination Shift-Enter einfügen. Die Nachricht die Sie verschickt haben wird ihnen vom server sofort wieder zurückgeschickt.
Wenn der Verbindungsversuch scheitert wird eine Nachricht mit dem Grund angezeigt.
Um eine Verbindung abzubrechen, geben Sie das Kommando "/disconnect" ins Texteingabefeld ein.
Um die Anwendung zu beenden, drücken Sie entweder das X in der oberen rechten Ecke des Fensters, wählen Sie File->Quit aus der Menüleiste oder geben Sie das Kommando "/exit" ein.

Eine Anmerkung zur Sicherheit:
ACHTUNG! nuke-ms bietet keine Möglichkeiten zur Verschlüsselung oder Authentifizierung! Die Identität eines Teilnehmers ist überhaupt nicht gewährleistet und die Nachrichten werden völlig unverschlüsselt über das Netzwerk geschickt und können daher abgehört werden.
Lassen Sie den Server nicht auf Sicherheitskritischen Geräten laufen, denn es ist wahrscheinlich sehr einfach verschiede Angriffe gegen ihn auszuführen. Bitte benutzen Sie nuke-ms nur in einer sicheren Umgebung (wie z.B. ihrem Heimnetzwerk hinter einer Firewall) und nur zum Testen.
Diese Probleme beruhen auf der Tatsache, dass in dieser frühen Entwicklungsphase noch keine Maßnahmen zur absicherung getroffen wurden. Sicherheit ist allerdings ein Hauptziel dieses Projekts und wird in Zukunft hoffentlich stark verbessert werden.


---------------------------------------------------------------------------------

4. Anmerkungen zur Kompatibilität

nuke-ms ist noch keine reife Software. Als solche ist auch das Kommunikationsprotokoll zwischen den Clients nicht reif und unterliegt andauernden Veränderungen. Kompatibilität zwischen verschiedenen Versionen ist in dieser frühen Phase der Entwicklung nicht gewährleistet. Tatsächlich ist eine inkompatible Änderung des Protokolls in naher zukunft geplant. Um also das Programm zur kommunikation zu verwenden sollten Sie immer die gleiche Version für jeden Teilnehmer verwenden.

---------------------------------------------------------------------------------

5. Hilfe, Kommentare, Fehlermeldungen und Patches

Ich bin sehr an jedem Feedback interessiert das Sie haben könnten. Sagen Sie mir, wie Sie nuke-ms finden!
Wenn Sie mit nuke-ms Hilfe brauchen, Fehler gefunden haben, das Programm geändert haben und mir einen Patch schicken wollen oder wenn Sie mich einfach kontaktieren wollen, senden Sie ein E-Mail an die nuke-ms Mailing Liste.
Die Adresse dieser Liste ist:
    nuke-ms-users@lists.berlios.de
Um E-Mails an diese Liste senden zu können müssen Sie sich einschreiben (keine Sorge, es kostet nichts und Sie werden keinen Spam bekommen außer den, den Sie selbst schicken ;-) ). Sie können sich auf dieser Seite einschreiben:
    https://lists.berlios.de/mailman/listinfo/nuke-ms-users

Wenn Sie mir Irgendetwas zu sagen haben von dem Sie glauben dass es  nicht für die Öffentlichkeit geeignet ist, mailen Sie mir an diese Addresse: fat.lobyte9@gmail.com
