import * as React from 'react';
import { Bounce, ToastContainer } from 'react-toastify';
import ImpTerm from './ImpTerm';

import 'react-toastify/dist/ReactToastify.css';

function App() {
  return (
    <div>
      <ImpTerm />
      <ToastContainer
        position="top-right"
        autoClose={3000}
        hideProgressBar={false}
        newestOnTop={false}
        closeOnClick
        rtl={false}
        pauseOnFocusLoss
        draggable
        pauseOnHover
        theme="colored"
        transition={Bounce}
      />
    </div>
  );
}

export default App;
