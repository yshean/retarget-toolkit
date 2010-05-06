using System;
using System.Collections.Generic;
using System.Text;
using TetComp;

namespace Eye_Tracker_Component_C_Sharp_NET
{
    class TetServiceEntryWrapper
    {
        private TetServiceEntry _entry;
        private string _servicename;

        public TetServiceEntryWrapper (TetServiceEntry entry)
	    {
            _entry = entry;
            _servicename = null;
	    }

        public TetServiceEntryWrapper(string servicename)
        {
            _servicename = servicename;
        }

        public override string ToString()
        {
            if (_servicename != null)
                return _servicename;

            string presentation;
            if (_entry.name != "" && _entry.name != _entry.model)
                presentation = _entry.name + " ";
            else
                presentation = _entry.pid + " ";
            presentation += "(" + _entry.model + ")";
            return presentation;
        }

        public override bool Equals(object obj)
        {
            TetServiceEntryWrapper wrapper = obj as TetServiceEntryWrapper;
            if (wrapper == null)
                return false;

            if (wrapper._servicename != null)
            {
                if (_servicename != null)
                    return wrapper._servicename == _servicename;
                else
                    return wrapper._servicename == _entry.servicename;
            }
            else
            {
                if (_servicename != null)
                    return wrapper._entry.servicename == _servicename;
                else
                    return _entry.servicename == wrapper._entry.servicename;
            }
        }

        public override int GetHashCode()
        {
            if (_servicename != null)
                return _entry.servicename.GetHashCode();
            return _servicename.GetHashCode();
        }

        public string Hostname
        {
            get { return _entry.hostname; }
        }

        public bool IsRunning
        {
            get {
                if (_servicename == null)
                    return _entry.status == TetServiceEntryStatus.TetServiceEntryStatus_RUNNING;
                return false;
            }
        }
    }
}
