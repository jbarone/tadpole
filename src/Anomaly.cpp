/*
 *   This file is part of TADpole.
 *
 *   TADpole is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   TADpole is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with TADpole.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Anomaly.h"

bool AnomalyPair::intersects (AnomalyPair *pair)
{
    return (
            (
            // Case:
            //      pair.start ------- pair.end
            //  this.start ------- this.end
            //
            ((this->m_previous->getNextEvent()->getDateCreated() >=
              pair->getPreviousAnomaly()->getNextEvent()->getDateCreated())
             && 
             (this->m_previous->getNextEvent()->getDateCreated() <
              pair->getPreviousAnomaly()->getNextEvent()->getDateCreated()))
            ||
            // Case:
            //  pair.start ------- pair.end
            //      this.start ------- this.end
            //
            ((this->m_next->getPreviousEvent()->getDateCreated() <=
              pair->getNextAnomaly()->getPreviousEvent()->getDateCreated())
             && 
             (this->m_next->getPreviousEvent()->getDateCreated() >
              pair->getPreviousAnomaly()->getNextEvent()->getDateCreated()))
            ||
            // Case:
            // pair.start ---------------- pair.end
            //      this.start ------ this.end
            ((this->m_previous->getNextEvent()->getDateCreated() <=
              pair->getPreviousAnomaly()->getNextEvent()->getDateCreated())
             &&
             (this->m_next->getPreviousEvent()->getDateCreated() >=
              pair->getNextAnomaly()->getPreviousEvent()->getDateCreated()))
            )
            ||
            (
            // Case:
            //      pair.start ------- pair.end
            //  this.start ------- this.end
            //
            ((this->m_previous->getNextEvent()->getDateWritten() >=
              pair->getPreviousAnomaly()->getNextEvent()->getDateWritten())
             && 
             (this->m_previous->getNextEvent()->getDateWritten() <
              pair->getPreviousAnomaly()->getNextEvent()->getDateWritten()))
            ||
            // Case:
            //  pair.start ------- pair.end
            //      this.start ------- this.end
            //
            ((this->m_next->getPreviousEvent()->getDateWritten() <=
              pair->getNextAnomaly()->getPreviousEvent()->getDateWritten())
             && 
             (this->m_next->getPreviousEvent()->getDateWritten() >
              pair->getPreviousAnomaly()->getNextEvent()->getDateWritten()))
            ||
            // Case:
            // pair.start ---------------- pair.end
            //      this.start ------ this.end
            ((this->m_previous->getNextEvent()->getDateWritten() <=
              pair->getPreviousAnomaly()->getNextEvent()->getDateWritten())
             &&
             (this->m_next->getPreviousEvent()->getDateWritten() >=
              pair->getNextAnomaly()->getPreviousEvent()->getDateWritten()))
            )
            );
}

void AnomalyCollection::addLog(LoggedAnomaly* log) 
{ 
    m_logs.push_back(log); 
    
    AnomalyPair *p = log->getPair();

    if (p->getPreviousAnomaly()->getPreviousEvent()->getDateCreated() >
            m_pair->getPreviousAnomaly()->getPreviousEvent()->getDateCreated())
        m_pair->getPreviousAnomaly()->getPreviousEvent()->setDateCreated(
                p->getPreviousAnomaly()->getPreviousEvent()->getDateCreated());
    if (p->getPreviousAnomaly()->getPreviousEvent()->getDateWritten() >
            m_pair->getPreviousAnomaly()->getPreviousEvent()->getDateWritten())
        m_pair->getPreviousAnomaly()->getPreviousEvent()->setDateWritten(
                p->getPreviousAnomaly()->getPreviousEvent()->getDateWritten());

    if (p->getPreviousAnomaly()->getNextEvent()->getDateCreated() <
            m_pair->getPreviousAnomaly()->getNextEvent()->getDateCreated())
        m_pair->getPreviousAnomaly()->getNextEvent()->setDateCreated(
                p->getPreviousAnomaly()->getNextEvent()->getDateCreated());
    if (p->getPreviousAnomaly()->getNextEvent()->getDateWritten() <
            m_pair->getPreviousAnomaly()->getNextEvent()->getDateWritten())
        m_pair->getPreviousAnomaly()->getNextEvent()->setDateWritten(
                p->getPreviousAnomaly()->getNextEvent()->getDateWritten());

    if (p->getNextAnomaly()->getPreviousEvent()->getDateCreated() >
            m_pair->getNextAnomaly()->getPreviousEvent()->getDateCreated())
        m_pair->getNextAnomaly()->getPreviousEvent()->setDateCreated(
                p->getNextAnomaly()->getPreviousEvent()->getDateCreated());
    if (p->getNextAnomaly()->getPreviousEvent()->getDateWritten() >
            m_pair->getNextAnomaly()->getPreviousEvent()->getDateWritten())
        m_pair->getNextAnomaly()->getPreviousEvent()->setDateWritten(
                p->getNextAnomaly()->getPreviousEvent()->getDateWritten());

    if (p->getNextAnomaly()->getNextEvent()->getDateCreated() <
            m_pair->getNextAnomaly()->getNextEvent()->getDateCreated())
        m_pair->getNextAnomaly()->getNextEvent()->setDateCreated(
                p->getNextAnomaly()->getNextEvent()->getDateCreated());
    if (p->getNextAnomaly()->getNextEvent()->getDateWritten() <
            m_pair->getNextAnomaly()->getNextEvent()->getDateWritten())
        m_pair->getNextAnomaly()->getNextEvent()->setDateWritten(
                p->getNextAnomaly()->getNextEvent()->getDateWritten());
};

void AnomalyCollection::addFile(file_info* file)
{
    m_files.push_back(file);
}
