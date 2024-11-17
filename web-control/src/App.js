import * as React from 'react';
import ImpTerm from './ImpTerm';
import { ToastContainer } from 'react-toastify';
import { Bounce } from 'react-toastify';

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
