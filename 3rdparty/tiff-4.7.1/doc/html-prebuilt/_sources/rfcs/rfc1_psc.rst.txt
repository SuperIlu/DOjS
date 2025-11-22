.. _rfc1_psc:

============================================
RFC 1: Project Steering Committee Guidelines
============================================

Author: Even Rouault

Contact: even.rouault at spatials.com

Status: Adopted

Summary
-------

This document describes how the libtiff Project Steering Committee
determines membership, and makes decisions on libtiff project issues.

In brief, the committee votes on proposals on the libtiff mailing list.
Proposals are available for review for at least 5 business days, and a single
veto is sufficient to delay progress though ultimately a majority of members can
pass a proposal.

Detailed Process
----------------

1.  Proposals are written up and submitted on the libtiff mailing list
    for discussion and voting, by any interested party, not just
    committee members.
2.  Proposals need to be available for review for at least 5 business
    days before a final decision can be made.
3.  Respondents may vote "+1" to indicate support for the proposal and a
    willingness to support implementation.
4.  Respondents may vote "-1" to veto a proposal, but must provide clear
    reasoning and alternate approaches to resolving the problem within
    the voting delay.
5.  A vote of -0 indicates mild disagreement, but has no effect. A 0
    indicates no opinion. A +0 indicate mild support, but has no effect.
6.  Anyone may comment on proposals on the list, but only members of the
    Project Steering Committee's votes will be counted.
7.  A proposal will be accepted if it receives at least +2 (including the
    proposer) and no vetos (-1).
8.  If a proposal is vetoed, and it cannot be revised to satisfy all
    parties, then it can be resubmitted for an override vote in which a
    majority of all eligible voters indicating +1 is sufficient to pass
    it. Note that this is a majority of all committee members, not just
    those who actively vote.
9.  Upon completion of discussion and voting the proposer should
    announce whether they are proceeding (proposal accepted) or are
    withdrawing their proposal (vetoed).
10. The Chair gets a vote.
11. The Chair is responsible for keeping track of who is a member of the
    Project Steering Committee.
12. Addition and removal of members from the committee, as well as
    selection of a Chair should be handled as a proposal to the
    committee.
13. The Chair adjudicates in cases of disputes about voting.

When is Vote Required?
----------------------

-  Anything that could cause backward compatibility issues.
-  Adding, removing or modifying substantial amounts of code.
-  Changing inter-subsystem APIs.
-  Issues of procedure.
-  When releases should take place.
-  Anything that might be controversial

Observations
------------

-  The Chair is the ultimate adjudicator if things break down.
-  The absolute majority rule can be used to override an obstructionist
   veto, but it is intended that in normal circumstances vetoers need to
   be convinced to withdraw their veto. We are trying to reach
   consensus.
-  It is strongly recommended that proposals subject to a vote be discussed
   first on the mailing list to build consensus before proceeding to the vote.

Bootstrapping
-------------

The initial members of the Project Steering Committee are:

- Bob Friesenhahn (@bobfriesenhahn)
- Su Laus (@Su_Laus)
- Roger Leigh (@rleigh)
- Timothy Lyanguzov (@theta682)
- Olivier Paquet (@1-Olivier)
- Leonard Rosenthol (@lrosenthol)
- Even Rouault (@rouault)

Even Rouault is declared initial Chair of the Project Steering Committee.

The current membership list can be found on the :ref:`psc` page.

Credits
-------

This document is a direct adaptation from
`GDAL Project Management Committee Guidelines <https://gdal.org/development/rfc/rfc1_pmc.html>`__,
authored by Frank Warmerdam.

Voting history
--------------

Adopted by +1 vote from all above mentioned initial members of the PSC.
