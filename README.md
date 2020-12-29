# Project
Εργασία για το μάθημα Ανάπτυξη Λογισμικού για Πληροφοριακά Συστήματα.  

Νικόλαος Νιώτης 1115201700106  
Θωμάς Παπαθυμιόπουλος 1115201700119

# Δομή  
Οι δομές δεδομένων που χρησιμοποιήθηκαν είναι οι εξής:  
- Ένα hashtable όπου κάθε bucket είναι ένας δείκτης σε ένα AVL.  
- AVL δέντρα των οποίων κάθε κόμβος δείχνει σε κάποιο entry και έχουν ως κριτήριο σύγκρισης το hashvalue του  δέντρου αυτού.  
- Κάθε entry αποτελείται από τα χαρακτηρηστικά του όπως page_title, id και hashvalue καθώς και δείκτη σε κάποια κλίκα και δείκτη σε parserlist με τα χαρακτηριστικά που επιστρέφει ο json parser.  
- Οι κλίκες είναι μια εξειδικευμένη λίστα από δείκτες σε entries. Κάθε entry κατατην δημιουργεία του συνοδεύεται από μια κλίκα που περιέχει μόνο τον εαυτό του.  
- Οι αντικλίκες είναι μια λίστα από δείκτες σε κλίκες. Κάθε κλίκα έχει την δικιά της αντικλίκα.  
- Τα dictionaries είναι AVL δέντρα των οποίων κάθε κόμβος αποτελεί μια λέξη. Κάθε dictionary έχει την δυνατότητα να επιστρέψε ιτον εαυτό του σε μορφή πίνακα με την συνάρτηση vecotrify.  
- Τα sparce matrices είναι ένας πίνακας από λίστες. Κάθε θέση του πίνακα αντιστοιχεί σε κάποιο αρχείο και η λίστα λάθε αρχείου περιέχει κόμβους από λέξεις όπου κάθε λέξη έχει φυλαγμένη τις vector συντεταγμένες της, την τιμή tfidf της για εκείνο το αρχείο και την τιμή bow για εκείνο το αρχείο. 
  
  ### Λεπτομέρειες  
  Όπως περιγράφεται παραπάνω υπάρχει μια κυκλική σχέση μεταξύ κλικών και entries με κάθε entry να δείχνει προς μια κλίκα και η κλίκα να δείχνει προς τα entries που δείχνουν σε αυτήν. Ως αποτέλεσμα για την αποφυγή του double free κατα την διαγραφή των δομών οι κλίκες δεν σβήνουν τα entries που περιλαμβάνουν, παρά μόνο αποσυνδέουν τον εαυτό τους από τα entries τους και σβήνουν τον εαυτό τους. Κάθε entry σβήνεται μέσω του destructor του AVL δέντρου.  Οι αντικλίκες καταστρέφονται παράλληλα με τις κλίκες. Οι πίνακες tfidf και bow φυλάσσονται σε μορφή sparce matrix για εξοικονόμηση μνήμης.

  # Parser  
  Ο parser είναι εξειδικευμένος για τα json που μας δίνονται στα dataset της εργασίας. Δηλαδή αποθηκεύει τα περιεχόμενα του json αρχείου μονάχα ως strings (στην περίπτωση πινάκων αποθηκεύεται το περιεχόμενο πάλι σε ένα string αλλά αυτή την φόρα γίνεται η χρήση του διαχωρηστικού χαρακτήρα '/t' για την διαφοροποίηση των στοιχείων του πίνακα) σε μια λίστα από ζευγάρια από var και content.  


  # Λογιστική Παλινδρόμηση
- Τα ζευγάρια του dataset W ανακατεύονται και έπειτα χωρίζονται σε 3 μερη (train, test, validation).  
- Από το train κομμάτι δημιουργούμε τις κλίκες και τις αντικλίκες και εξάγουμε την πληροφορία που δημιουργούμε από αυτές τις σχέσεις στο output.csv.
- Tο training dataset του lr αποσπάται από το output.csv
- Το training γίνεται σε batches (δηλαδή τα βάρη ανανεώνονται στο τέλος κάθε iteration, όπου το κάθε iteration τρέχει για όλα τα ζευγάρια του output.csv).
- Το validation βγάζει accuracy score για τα ζευγάρια που αποσπάστηκαν από το W και έπειτα βρίσκει το πιο ταιριαστό entry για κάθε entry που δεν ανήκει σε κάποια κλίκα.   

  # Εκτέλεση
  Για το compilation της εργασίας: ```make```   
  Για την εκτέλεση της εργασίας: ```./project1```  
  Για το compilation κάποιου unit test: ```make unit_test```  
  Για την εκτέλεση των unit test:  ```./unit_test```